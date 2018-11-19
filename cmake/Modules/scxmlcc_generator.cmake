function (scxmlcc_generator filename gen-list )

   # if we didn't compile the scxmlcc, then go find it in the system
   if ( TARGET scxmlcc )
      set(SCC scxmlcc )
   else()
      find_program( SCC scxmlcc )
   endif()
   
   get_filename_component(ext ${filename} EXT)
   if(NOT ext STREQUAL ".scxml")
      message("skipping ${filename}") 
      return()
   endif()

   get_filename_component(base ${filename} NAME_WE)
   set(output ${CMAKE_CURRENT_BINARY_DIR}/${base}.h)
   
   set_source_files_properties(${output} PROPERTIES GENERATED TRUE)
   set(${gen-list} ${${gen-list}} ${output} PARENT_SCOPE)
   
   add_custom_command(
        OUTPUT ${output}
        COMMAND ${SCC} --stringevents -i ${filename}  -o ${output} 
        DEPENDS ${filename}
   )

endfunction()
