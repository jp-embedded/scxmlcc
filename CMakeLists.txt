cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules )
add_subdirectory(src)

# prevent the examples and tests libs installing
macro(install)
endmacro()

add_subdirectory(src/examples)

enable_testing()
add_subdirectory(src/test)
