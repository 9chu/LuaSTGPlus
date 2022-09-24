function(lstg_gen_perfect_hasher)
    find_package(Python3 COMPONENTS Interpreter)

    if(NOT Python3_Interpreter_FOUND)
        message(FATAL "Python3 is required to build this project")
    endif()

    set(ONE_VALUE_ARGS DECL OUT_HEADER OUT_SOURCE)
    cmake_parse_arguments(HPERF "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    set(COMMAND_LINE --define "${HPERF_DECL}" --header "${HPERF_OUT_HEADER}" --source "${HPERF_OUT_SOURCE}")

    # message(STATUS ${COMMAND_LINE})

    add_custom_command(
        OUTPUT "${HPERF_OUT_HEADER}" "${HPERF_OUT_SOURCE}"
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tool/PerfectHashTool/PerfectHashTool.py ${COMMAND_LINE}
        DEPENDS ${HPERF_DECL} "${CMAKE_SOURCE_DIR}/tool/PerfectHashTool/PerfectHashTool.py"
        COMMENT "Running perfect hash tool" VERBATIM )

    set_source_files_properties("${HPERF_OUT_HEADER}" PROPERTIES GENERATED TRUE)
    set_source_files_properties("${HPERF_OUT_SOURCE}" PROPERTIES GENERATED TRUE)
endfunction()
