cmake_minimum_required(VERSION 2.8.11)
project( scxmlcc )

find_package (Boost REQUIRED COMPONENTS filesystem system program_options )

add_executable( ${PROJECT_NAME}
   cpp_output.cpp
   main.cpp
   scxml_parser.cpp
   version_git.cpp
   )

target_include_directories( ${PROJECT_NAME} PRIVATE 
   ${CMAKE_BINARY_DIR} 
   ${Boost_INCLUDE_DIR} )

target_link_libraries( ${PROJECT_NAME} PRIVATE
   ${Boost_LIBRARIES} )

set_property(TARGET ${PROJECT_NAME} 
   PROPERTY CXX_STANDARD 11)

install(TARGETS ${PROJECT_NAME} 
   DESTINATION bin)

install(FILES   ${CMAKE_SOURCE_DIR}/cmake/Modules/scxmlcc_generator.cmake 
    DESTINATION share/cmake/Modules )

# create version header file
find_package(Git)
execute_process(
  COMMAND git describe --first-parent --tags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_DESCRIBE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
FILE(WRITE ${CMAKE_BINARY_DIR}/gitversion.h "\#define GITVERSION \"${GIT_DESCRIBE}\"\n")

