# Generate statemachine from input file
# Usage:
#   INPUTS: Input file(s)
#   TARGETS: Name of target(s), which depends on the statemachine
#   CLIARGUMENTS: Optional scxmlcc cli argument(s) (e.g. --debug=clog and/or --cpp14)
#
# The generated file(s) are placed within current build folder. Every input
# file will produce an output file. The output name depends on the input name,
# eg. microwave-01-cplusplus.xml --> microwave-01-cplusplus.h
#
# Be aware: You can't use the scxmlcc_generator command with the same INPUTS arguments twice, e.g.:
# scxmlcc_generator(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/microwave-02-cplusplus.scxml TARGETS testlib)
# scxmlcc_generator(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/microwave-02-cplusplus.scxml TARGETS testlib2)
#
# Instead use a single command and specify both targets:
# scxmlcc_generator(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/microwave-02-cplusplus.scxml TARGETS testlib testlib2)

function (scxmlcc_generator )

  set(options "")
  set(oneValueArgs "")
  set(multiValueArgs INPUTS TARGETS CLIARGUMENTS)

  cmake_parse_arguments(PARSED_ARG
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
  )

   # if we didn't compile the scxmlcc, then go find it in the system
   if ( TARGET scxmlcc )
      set(SCC scxmlcc )
   else()
      find_program( SCC scxmlcc )
   endif()

   if(NOT PARSED_ARG_INPUTS)
     message(FATAL_ERROR "Missing INPUTS parameter for scxmlcc_generator command")
   endif()

   foreach(input ${PARSED_ARG_INPUTS})
     get_filename_component(ext ${input} EXT)
     if(NOT ext STREQUAL ".scxml")
        message("skipping ${input}")
        continue()
     endif()

     get_filename_component(base ${input} NAME_WE)
     set(output ${CMAKE_CURRENT_BINARY_DIR}/${base}.h)

     add_custom_command(
          OUTPUT ${output}
          COMMAND ${SCC} ${PARSED_ARG_CLIARGUMENTS} --stringevents -i ${input}  -o ${output}
          DEPENDS ${input}
     )
     list(APPEND outputs ${output})
   endforeach()

   # Append hash to avoid problems with same named input files
   string(MD5 hash "${PARSED_ARG_INPUTS}")
   set(intermediateTarget "scxmlcc_${base}_${hash}")

   add_custom_target(
        ${intermediateTarget}
        DEPENDS ${outputs}
   )

   foreach(target ${PARSED_ARG_TARGETS})
      add_dependencies(${target} ${intermediateTarget})
   endforeach()



endfunction()
