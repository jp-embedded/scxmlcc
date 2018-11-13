cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules )

add_subdirectory(src)

if( DEFINED DEVELOPER ) 
   add_subdirectory(src/examples)
   enable_testing()
   add_subdirectory(src/test)
endif( DEFINED DEVELOPER ) 
