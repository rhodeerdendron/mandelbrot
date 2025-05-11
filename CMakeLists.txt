cmake_minimum_required(VERSION 3.22)

project(mandelbrot LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")



add_compile_options(-O2)

add_compile_options(-v)
add_compile_options(-fdiagnostics-color=always)

add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_options(-Wfloat-equal -Wpointer-arith -Wunreachable-code -Wmissing-field-initializers)
add_compile_options(-Werror=odr)

add_compile_options(-fstack-protector-strong)
add_compile_options(-Wformat=2 -Werror=format-security)
add_compile_options(-Wbidi-chars=any,ucn)
add_compile_options(-fno-delete-null-pointer-checks)

# TODO debug vs release
add_compile_options(-g -ggdb3)
#add_compile_options(-Werror)



string(COMPARE EQUAL "${CMAKE_CXX_COMPILER_ID}" "Clang" CC_CLANG)
string(COMPARE EQUAL "${CMAKE_CXX_COMPILER_ID}" "GNU"   CC_GCC)
string(COMPARE EQUAL "${CMAKE_CXX_COMPILER_ID}" "Intel" CC_INTEL)
string(COMPARE EQUAL "${CMAKE_CXX_COMPILER_ID}" "MSVC"  CC_MSVC)


option(ENABLE_FAST_MATH "Enable fast math optimizations" ON)
if(ENABLE_FAST_MATH)
    message(STATUS "Building with -ffast-math")

    if(CC_CLANG OR CC_GCC)
		add_compile_options(-ffast-math)
    elseif(CC_INTEL)
        add_compile_options(-fp-model fast=1) # FIXME wrong? fast=2?
    elseif(CC_MSVC)
		add_compile_options(/fp:fast)
	endif()
else()
    if(CC_INTEL)
        add_compile_options(-fp-model precise)
    elseif(CC_MSVC)
		add_compile_options(/fp:precise)
	endif()
endif()



add_executable(
    mandelbrot
    src/main.cpp
    src/vertex-array.cpp
    src/program.cpp
    src/texture.cpp
    src/rendertarget.cpp
    src/screen.cpp
    src/text.cpp)

target_include_directories(mandelbrot PRIVATE src)



# libraries

message(STATUS "Locating libraries")

# tell SDL to build its own dependencies
set(SDL_VENDORED ON)
set(SDLTTF_VENDORED ON)

# glew requires an extra step to download source
execute_process(COMMAND make extensions WORKING_DIRECTORY glew)

add_subdirectory(SDL EXCLUDE_FROM_ALL)
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)
add_subdirectory(glew/build/cmake EXCLUDE_FROM_ALL)

if(TARGET glew AND NOT TARGET glew::glew)
    add_library(glew::glew ALIAS glew)
endif()
if(NOT TARGET glew::glew)
    message(FATAL_ERROR "could not locate GLEW library")
endif()

# opengl is device-specific, cannot be downloaded
find_package(OpenGL REQUIRED)
#target_include_directories(mandelbrot PRIVATE ${OPENGL_INCLDUE_DIRS})

message(STATUS "Located all libraries")



# sanitizers

set(ENABLE_SANITIZER "NONE" CACHE STRING "Enable sanitizer")
set_property(CACHE ENABLE_SANITIZER PROPERTY STRINGS "NONE" "UNDEFINED" "ADDRESS")

if(ENABLE_SANITIZER STREQUAL "NONE" OR NOT ENABLE_SANITIZER)
    message(VERBOSE "Using no sanitizers")

elseif(ENABLE_SANITIZER STREQUAL "UNDEFINED")
    message(STATUS "Using -fsanitize=undefined")
    add_compile_options(-fsantize=undefined,bounds -fno-sanitize=bool -fno-sanitize-recover=all)
    add_link_options(-fsanitize=undefined)

elseif(ENABLE_SANITIZER STREQUAL "ADDRESS")
    message(STATUS "Using -fsanitize=address")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
    add_compile_definitions(_FORTIFY_SOURCE=0)
    # asan does not work with pie/pic
    include(CheckPIESupported)
    check_pie_supported()
    set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE FALSE)

else()
    message(FATAL_ERROR "Unknown ENABLE_SANITIZER value '${ENABLE_SANITIZER}'")

endif()



# enable lto

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_result OUTPUT lto_error LANGUAGES CXX)

if(lto_result)
    message(STATUS "Using IPO / LTO")
    set_property(TARGET mandelbrot PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    target_compile_options(mandelbrot PRIVATE -Werror=lto-type-mismatch)
else()
    message(WARNING "IPO / LTO unsupported, error: <${lto_error}>")
endif()



target_link_libraries(mandelbrot
    OpenGL::GL
    glew::glew
    SDL2_ttf::SDL2_ttf
    SDL2::SDL2)
