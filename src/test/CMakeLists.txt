cmake_minimum_required(VERSION 2.8.11)

project(tests)

############################################
# per gtest instructions for cmake inclusion
# https://github.com/google/googletest/blob/master/googletest/README.md

configure_file(gtest.cmake.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_CXX_STANDARD 11 ) 


############################################
# Setup scxml tests
#
# generate  txml->scxml->headers

include( scxmlcc_generator )
file(GLOB txmls "test*.txml")
find_program( XSLT xsltproc )
foreach(file ${txmls})
   get_filename_component(base ${file} NAME_WE)
   set(output ${CMAKE_CURRENT_BINARY_DIR}/${base}.scxml)
   add_custom_command(
       OUTPUT ${output}
       COMMAND ${XSLT} ${CMAKE_CURRENT_LIST_DIR}/cpp.xsl ${file} > ${output}
       DEPENDS ${file}
       )
   scxmlcc_generator( ${output} gen_src )
endforeach()

#generate  scxml->headers
scxmlcc_generator( ${CMAKE_CURRENT_SOURCE_DIR}/event_list.scxml gen_src )
scxmlcc_generator( ${CMAKE_CURRENT_SOURCE_DIR}/event_tokens.scxml gen_src )
scxmlcc_generator( ${CMAKE_CURRENT_SOURCE_DIR}/conditional.scxml gen_src )
scxmlcc_generator( ${CMAKE_CURRENT_SOURCE_DIR}/lcca_parallel.scxml gen_src )
scxmlcc_generator( ${CMAKE_CURRENT_SOURCE_DIR}/issue_69.scxml gen_src )
scxmlcc_generator( ${CMAKE_CURRENT_SOURCE_DIR}/issue_72.scxml gen_src )
scxmlcc_generator( ${CMAKE_CURRENT_SOURCE_DIR}/eventless.scxml gen_src )

add_executable( test_scxml
    ${gen_src}
    test.cpp
    test_t.cpp
    )

target_link_libraries(test_scxml 
	gtest_main
    )

add_test(scxml_tests test_scxml)
