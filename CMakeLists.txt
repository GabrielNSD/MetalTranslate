cmake_minimum_required(VERSION 3.10)

project(MetalTranslate)

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

# Set ICU root directory (where Homebrew installed ICU)
set(ICU_ROOT "/opt/homebrew/opt/icu4c")

# Manually specify the include and library directories for ICU
set(ICU_INCLUDE_DIR ${ICU_ROOT}/include)
set(ICU_LIBRARY_DIR ${ICU_ROOT}/lib)

# Find and link ICU
find_package(ICU REQUIRED COMPONENTS uc i18n data)
include_directories(${ICU_INCLUDE_DIR})

ExternalProject_Add(CTranslate2
    PREFIX third_party/CTranslate2
    GIT_REPOSITORY https://github.com/OpenNMT/CTranslate2
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DWITH_MKL=OFF -DWITH_DNNL=ON -DOPENMP_RUNTIME=NONE
)

ExternalProject_Add(Tokenizer
    PREFIX third_party/Tokenizer
    GIT_REPOSITORY https://github.com/OpenNMT/Tokenizer
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
    -DCMAKE_CXX_FLAGS="-Wno-enum-constexpr-conversion"
    CXX_STANDARD 11
)

# Set oneDNN root directory (update this to your actual oneDNN installation directory)
set(ONEDNN_ROOT "/opt/homebrew/Cellar/onednn/3.4.2")

# Include oneDNN directory and library
include_directories(${ONEDNN_ROOT}/include)
link_directories(${ONEDNN_ROOT}/lib)

# -DCMAKE_CXX_FLAGS="-Wno-enum-constexpr-conversion"

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_library(metaltranslate SHARED
    src/MetalTranslate.cpp
    src/metal_api.cpp
)

# add_executable(metaltranslate src/main.cpp)
set(TARGET_H
    src/MetalTranslate.h
    src/MetalTranslateConfig.h
    src/metal_api.h
)

# Include directories
target_include_directories(metaltranslate PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/CTranslate2
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Tokenizer
    ${CMAKE_BINARY_DIR}/third_party/CTranslate2
    ${CMAKE_BINARY_DIR}/third_party/Tokenizer
)

# target_sources(metaltranslate PRIVATE src/MetalTranslate.cpp)
# add_subdirectory(third_party/CTranslate2)

# target_link_libraries(metaltranslate ctranslate2)
# add_subdirectory(third_party/Tokenizer)

# target_link_libraries(metaltranslate OpenNMTTokenizer)

# Link third-party libraries to the metaltranslate library
target_link_libraries(metaltranslate PRIVATE ctranslate2 OpenNMTTokenizer dnnl icuuc)

# Ensure rpath is set to include the directory where dependencies will be located
set_target_properties(metaltranslate PROPERTIES
    CXX_EXTENSIONS NO
    CXX_STANDARD 17
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
)

set_target_properties(metaltranslate PROPERTIES
    CXX_EXTENSIONS NO
    CXX_STANDARD 17
)

# Copy required dylibs to the build directory
add_custom_command(TARGET metaltranslate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${ONEDNN_ROOT}/lib/libdnnl.3.4.dylib $<TARGET_FILE_DIR:metaltranslate>
    COMMAND ${CMAKE_COMMAND} -E copy ${ICU_LIBRARY_DIR}/libicuuc.74.dylib $<TARGET_FILE_DIR:metaltranslate>
)