#[==[
cmake_minimum_required(VERSION 3.10)

project(MetalTranslate)

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

ExternalProject_Add(CTranslate2
    GIT_REPOSITORY https://github.com/OpenNMT/CTranslate2
    GIT_TAG v4.3.1
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DWITH_MKL=OFF -DWITH_DNNL=ON -DOPENMP_RUNTIME=COMP
)

ExternalProject_Add(Tokenizer
    GIT_REPOSITORY https://github.com/OpenNMT/Tokenizer
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(metaltranslate src/main.cpp)

set(TARGET_H
    src/MetalTranslate.h
    src/MetalTranslateConfig.h
    src/metal_api.h
)

target_sources(metaltranslate PRIVATE src/MetalTranslate.cpp src/metal_api.cpp)

#add_subdirectory(third_party/CTranslate2/)
target_link_libraries(metaltranslate ctranslate2)

##add_subdirectory(third_party/Tokenizer/)
target_link_libraries(metaltranslate OpenNMTTokenizer)

#target_link_libraries(metaltranslate cpu_features)
]==]
cmake_minimum_required(VERSION 3.13)

project(MetalTranslate)

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

if(APPLE)
    # Set ICU root directory (where Homebrew installed ICU)
    execute_process(
        COMMAND brew --prefix icu4c
        OUTPUT_VARIABLE ICU_ROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Manually specify the include and library directories for ICU
    set(ICU_INCLUDE_DIR ${ICU_ROOT}/include)
    set(ICU_LIBRARY_DIR ${ICU_ROOT}/lib)

    # Find and link ICU
    find_package(ICU REQUIRED COMPONENTS uc i18n data)
    include_directories(${ICU_INCLUDE_DIR})

    # Execute brew command to get the ONEDNN_ROOT
    execute_process(
        COMMAND brew --prefix onednn
        OUTPUT_VARIABLE ONEDNN_ROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Include oneDNN directory and library
    include_directories(${ONEDNN_ROOT}/include)
elseif(UNIX)
    # find libdnnl.so
    execute_process(
        COMMAND dpkg --listfiles libdnnl-dev
        COMMAND grep libdnnl.so
        OUTPUT_VARIABLE LIBDNNL_SO
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

if(APPLE)
    ExternalProject_Add(CTranslate2
        PREFIX third_party/CTranslate2
        GIT_REPOSITORY https://github.com/OpenNMT/CTranslate2
        GIT_TAG v4.3.1
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DWITH_MKL=OFF -DWITH_DNNL=ON -DOPENMP_RUNTIME=NONE
        INSTALL_RPATH "@loader_path"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
elseif(UNIX)
    ExternalProject_Add(CTranslate2
        PREFIX third_party/CTranslate2
        GIT_REPOSITORY https://github.com/OpenNMT/CTranslate2
        GIT_TAG v4.3.1
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DWITH_MKL=OFF -DWITH_DNNL=ON -DOPENMP_RUNTIME=NONE
        INSTALL_RPATH "$$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
else()
    ExternalProject_Add(CTranslate2
        PREFIX third_party/CTranslate2
        GIT_REPOSITORY https://github.com/OpenNMT/CTranslate2
        GIT_TAG v4.3.1
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DWITH_MKL=OFF -DMKL_CBWR=COMPATIBLE -DWITH_DNNL=ON -DOPENMP_RUNTIME=COMP
    )
endif()

if(UNIX)
    ExternalProject_Add(Tokenizer
        PREFIX third_party/Tokenizer
        GIT_REPOSITORY https://github.com/OpenNMT/Tokenizer
        GIT_TAG v1.37.1
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
        -DCMAKE_CXX_FLAGS="-Wno-enum-constexpr-conversion"
        CXX_STANDARD 11
    )
else()
    ExternalProject_Add(Tokenizer
        PREFIX third_party/Tokenizer
        GIT_REPOSITORY https://github.com/OpenNMT/Tokenizer
        GIT_TAG v1.37.1
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
        CXX_STANDARD 11
    )
endif()

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_library(metaltranslate SHARED
    src/MetalTranslate.cpp
    src/metal_api.cpp
)

target_include_directories(metaltranslate PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${EXTERNAL_INSTALL_LOCATION}/include
)

# target_compile_definitions(metaltranslate PRIVATE METALTRANSLATE_EXPORTS)

add_dependencies(metaltranslate CTranslate2 Tokenizer)

if(WIN32)
    add_definitions(-DMETALTRANSLATE_EXPORTS)
endif()

message("DIRS: ")
message(STATUS "${EXTERNAL_INSTALL_LOCATION}")
message(STATUS "${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "${LIBDNNL_SO}")

target_link_libraries(metaltranslate PRIVATE ctranslate2 OpenNMTTokenizer)

# Ensure rpath is set to include the directory where dependencies will be located
if(APPLE)
    set_target_properties(metaltranslate PROPERTIES
        CXX_EXTENSIONS NO
        CXX_STANDARD 17
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        INSTALL_RPATH "@loader_path"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
elseif(UNIX)
    set_target_properties(metaltranslate PROPERTIES
        CXX_EXTENSIONS NO
        CXX_STANDARD 17
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
else()
    set_target_properties(metaltranslate PROPERTIES
        CXX_EXTENSIONS NO
        CXX_STANDARD 17
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

if(APPLE)
    # Set RPATH for all binaries
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    list(APPEND CMAKE_INSTALL_RPATH "@loader_path")
endif()

# TODO: find a better way to get these libraries version number (they can change between builds)
if(APPLE)
    # Copy required dylibs to the build directory and adjust LC_LOAD_DYLIB path
    add_custom_command(TARGET metaltranslate POST_BUILD
        COMMAND install_name_tool -change ${ONEDNN_ROOT}/lib/libdnnl.3.dylib @rpath/libdnnl.3.dylib ${EXTERNAL_INSTALL_LOCATION}/lib/libctranslate2.4.3.1.dylib # change lib path of ctranslate2 lib
        COMMAND install_name_tool -change ${ICU_LIBRARY_DIR}/libicuuc.74.dylib @rpath/libicuuc.74.dylib ${EXTERNAL_INSTALL_LOCATION}/lib/libctranslate2.4.3.1.dylib
        COMMAND install_name_tool -change ${ICU_LIBRARY_DIR}/libicudata.74.dylib @rpath/libicudata.74.dylib ${EXTERNAL_INSTALL_LOCATION}/lib/libOpenNMTTokenizer.dylib
        COMMAND install_name_tool -change ${ICU_LIBRARY_DIR}/libicuuc.74.dylib @rpath/libicuuc.74.dylib ${EXTERNAL_INSTALL_LOCATION}/lib/libOpenNMTTokenizer.dylib

        # copy files to build directory
        COMMAND ${CMAKE_COMMAND} -E copy ${ONEDNN_ROOT}/lib/libdnnl.3.5.dylib $<TARGET_FILE_DIR:metaltranslate>
        COMMAND ${CMAKE_COMMAND} -E copy ${ICU_LIBRARY_DIR}/libicuuc.74.2.dylib $<TARGET_FILE_DIR:metaltranslate>
        COMMAND ${CMAKE_COMMAND} -E copy ${ICU_LIBRARY_DIR}/libicudata.74.2.dylib $<TARGET_FILE_DIR:metaltranslate>
        COMMENT "Adjusting LC_LOAD_DYLIB path for libdnnl"
    )
elseif(UNIX)
    add_custom_command(TARGET metaltranslate POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy ${EXTERNAL_INSTALL_LOCATION}/lib/libctranslate2.so $<TARGET_FILE_DIR:metaltranslate>/libctranslate2.so.4
        COMMAND ${CMAKE_COMMAND} -E copy ${EXTERNAL_INSTALL_LOCATION}/lib/libOpenNMTTokenizer.so $<TARGET_FILE_DIR:metaltranslate>
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBDNNL_SO} $<TARGET_FILE_DIR:metaltranslate>

        COMMAND patchelf --replace-needed libdnnl.so.3 '$$ORIGIN/libdnnl.so' ${CMAKE_CURRENT_BINARY_DIR}/libctranslate2.so.4
        COMMENT "Adjusting ctranslate2 rpath and copying shared objects"
    )
else()
    add_custom_command(TARGET metaltranslate POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:metaltranslate> $<TARGET_RUNTIME_DLLS:metaltranslate>
        COMMAND dir /s $<TARGET_FILE_DIR:metaltranslate>
        COMMAND_EXPAND_LISTS
    )
endif()