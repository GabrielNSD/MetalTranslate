cmake_minimum_required(VERSION 3.10)

project(MetalTranslate)

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
)

# target_sources(metaltranslate PRIVATE src/MetalTranslate.cpp)
add_subdirectory(third_party/CTranslate2)

# target_link_libraries(metaltranslate ctranslate2)
add_subdirectory(third_party/Tokenizer)

# target_link_libraries(metaltranslate OpenNMTTokenizer)

# Link third-party libraries to the metaltranslate library
target_link_libraries(metaltranslate PRIVATE ctranslate2 OpenNMTTokenizer)

set_target_properties(metaltranslate PROPERTIES
    CXX_EXTENSIONS NO
)