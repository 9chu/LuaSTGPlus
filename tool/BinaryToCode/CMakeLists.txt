function(lstg_binary_to_code)
    find_package(Python3 COMPONENTS Interpreter)

    if(NOT Python3_Interpreter_FOUND)
        message(FATAL "Python3 is required to build this project")
    endif()

    set(ONE_VALUE_ARGS INPUT NAME OUTPUT)
    cmake_parse_arguments(BINARY_TO_CODE "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    set(COMMAND_LINE --i "${BINARY_TO_CODE_INPUT}" -n "${BINARY_TO_CODE_NAME}" -o "${BINARY_TO_CODE_OUTPUT}")

    get_filename_component(BINARY_TO_CODE_TOOL_SOURCE_DIR "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" DIRECTORY CACHE)

    add_custom_command(
        OUTPUT "${BINARY_TO_CODE_OUTPUT}"
        COMMAND ${Python3_EXECUTABLE} "${BINARY_TO_CODE_TOOL_SOURCE_DIR}/BinaryToCode.py" ${COMMAND_LINE}
        DEPENDS ${BINARY_TO_CODE_INPUT} "${BINARY_TO_CODE_TOOL_SOURCE_DIR}/BinaryToCode.py"
        COMMENT "Running binary to code tool" VERBATIM)

    set_source_files_properties("${BINARY_TO_CODE_OUTPUT}" PROPERTIES GENERATED TRUE)
endfunction()
