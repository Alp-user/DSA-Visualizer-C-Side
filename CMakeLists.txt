cmake_minimum_required(VERSION 3.10)
project(MyProject C CXX)

# Set the source and include directories
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Find all source files
file(GLOB_RECURSE SOURCE_FILES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c")

# Add the include directory to the include path for all targets
include_directories(${INCLUDE_DIR})

# Create the executable from the source files
add_executable(program ${SOURCE_FILES})
# Add compile flags and include directories

target_compile_options(program PUBLIC
    -g
    -Wall
    -Wextra
    -std=c99
    -std=c++17
)

# Add the includes using a proper command for CMake
target_include_directories(program PUBLIC
    ${INCLUDE_DIR}
    /usr/include/freetype2
    /usr/include/libpng16
    /usr/include/harfbuzz
    /usr/include/glib-2.0
    /usr/lib/glib-2.0/include
    /usr/include/sysprof-6
)

# Add link libraries
target_link_libraries(program PUBLIC
    GL
    glfw
    freetype
)
