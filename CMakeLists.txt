cmake_minimum_required(VERSION 3.12)

project(MetalTranslate)

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

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
link_directories(${ONEDNN_ROOT}/lib)

ExternalProject_Add(CTranslate2
    PREFIX third_party/CTranslate2
    GIT_REPOSITORY https://github.com/OpenNMT/CTranslate2
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DWITH_MKL=OFF -DWITH_DNNL=ON -DOPENMP_RUNTIME=NONE
)

ExternalProject_Add_Step(CTranslate2 set_rpath
    COMMAND ${CMAKE_COMMAND} -E echo "Setting RPATH for CTranslate2"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_INSTALL_LOCATION}/lib
    COMMAND install_name_tool -add_rpath "@loader_path" ${EXTERNAL_INSTALL_LOCATION}/lib/libctranslate2.dylib
    DEPENDEES install
    ALWAYS 1
)

ExternalProject_Add(Tokenizer
    PREFIX third_party/Tokenizer
    GIT_REPOSITORY https://github.com/OpenNMT/Tokenizer
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
    -DCMAKE_CXX_FLAGS="-Wno-enum-constexpr-conversion"
    CXX_STANDARD 11
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_library(metaltranslate SHARED
    src/MetalTranslate.cpp
    src/metal_api.cpp
)

set(TARGET_H
    src/MetalTranslate.h
    src/MetalTranslateConfig.h
    src/metal_api.h
)

target_include_directories(metaltranslate PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${EXTERNAL_INSTALL_LOCATION}/include
)

add_dependencies(metaltranslate CTranslate2 Tokenizer)
target_link_libraries(metaltranslate PRIVATE ctranslate2 OpenNMTTokenizer dnnl icuuc)

# Ensure rpath is set to include the directory where dependencies will be located
set_target_properties(metaltranslate PROPERTIES
    CXX_EXTENSIONS NO
    CXX_STANDARD 17
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# Set RPATH for all binaries
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(APPEND CMAKE_INSTALL_RPATH "@loader_path")

# Copy required dylibs to the build directory
add_custom_command(TARGET metaltranslate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${ONEDNN_ROOT}/lib/libdnnl.3.4.dylib $<TARGET_FILE_DIR:metaltranslate>
    COMMAND ${CMAKE_COMMAND} -E copy ${ICU_LIBRARY_DIR}/libicuuc.74.2.dylib $<TARGET_FILE_DIR:metaltranslate>
)