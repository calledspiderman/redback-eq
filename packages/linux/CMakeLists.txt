cmake_minimum_required(VERSION 3.20)
project(radioform-linux VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
    add_compile_options($<$<CONFIG:Release>:-O3>)
    add_compile_options($<$<CONFIG:Release>:-ffast-math>)
    add_compile_options($<$<CONFIG:Release>:-fno-finite-math-only>)
    add_compile_options($<$<CONFIG:Debug>:-g>)
    add_compile_options($<$<CONFIG:Debug>:-O0>)
    add_compile_options($<$<CONFIG:Debug>:-fsanitize=address>)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_link_options(-fsanitize=address)
    endif()
endif()

set(RADIOFORM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(DSP_SOURCE_DIR "${RADIOFORM_SOURCE_DIR}/packages/dsp")

add_subdirectory(${DSP_SOURCE_DIR} dsp_build)

option(BUILD_FILTER "Build PipeWire EQ filter" ON)
option(BUILD_UI "Build GTK3 UI app" ON)

if(BUILD_FILTER)
    add_subdirectory(src/filter radioform-filter)
endif()

if(BUILD_UI)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../apps/linux linux-ui)
endif()
