cmake_minimum_required(VERSION 2.8.11)
project( scxmlcc )

find_package (Boost REQUIRED COMPONENTS filesystem system program_options )

add_executable( ${PROJECT_NAME}
   cpp_output.cpp
   dot_output.cpp
   main.cpp
   scxml_parser.cpp
   version.cpp
   ${CMAKE_BINARY_DIR}/version_auto.h
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

if(UNIX)
   # create version header - in build folder
   ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_BINARY_DIR}/version_auto.h
     COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && autorevision -tsh > ${CMAKE_BINARY_DIR}/autorevision.mk
     COMMAND cd ${CMAKE_BINARY_DIR} && sh ${CMAKE_CURRENT_SOURCE_DIR}/makerevision.sh || truncate -s0 version_auto.h
   )
else()
   # do something useful on windows
   FILE(WRITE ${CMAKE_BINARY_DIR}/version_auto.h "")
endif()
