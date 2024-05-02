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
set( CMAKE_CXX_STANDARD 17 )


############################################
# Setup scxml tests
#
# generate  txml->scxml->headers

include( scxmlcc_generator )

#    test_t.cpp currently disabled because dies not work with scxmlcc_generator
add_executable( test_scxml
    test.cpp
    )

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
     scxmlcc_generator(INPUTS ${output} TARGETS test_scxml)
endforeach()

#generate  scxml->headers
scxmlcc_generator(
  INPUTS
    ${CMAKE_CURRENT_SOURCE_DIR}/event_list.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/event_tokens.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/conditional.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/lcca_parallel.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/issue_69.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/issue_72.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/issue_88.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/issue_94.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/issue_97.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/eventless.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/stringevents.scxml
    ${CMAKE_CURRENT_SOURCE_DIR}/parallel_exit.scxml
  TARGETS
    test_scxml
  )


target_link_libraries(test_scxml
	gtest_main
    )

add_test(scxml_tests test_scxml)
