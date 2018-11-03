cmake_minimum_required(VERSION 2.8.11)

project(tests)

# setup gtest
add_subdirectory(gtest)
enable_testing()

set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_CXX_STANDARD 11 ) 

#generate  txml->scxml->headers
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

# includes and libs
target_link_libraries( test_scxml gtest gtest_main )
target_include_directories( test_scxml PRIVATE
    ${gtest_SOURCE_DIR}/include 
    ${gtest_SOURCE_DIR}
    )

add_test(txml_tests test_scxml)
