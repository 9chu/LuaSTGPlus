function(lstg_gen_auto_bridge_cpp)
    find_package(Python3 COMPONENTS Interpreter)

    if(NOT Python3_Interpreter_FOUND)
        message(FATAL "Python3 is required to build this project")
    endif()

    set(ONE_VALUE_ARGS OUTPUT NAME BASE)
    set(MULTI_VALUE_ARGS NAMESPACE FILES)
    cmake_parse_arguments(BRIDGE_CPP "" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    set(COMMAND_LINE --output "${BRIDGE_CPP_OUTPUT}" --name "${BRIDGE_CPP_NAME}" --file ${BRIDGE_CPP_FILES})
    if(DEFINED BRIDGE_CPP_BASE)
        list(APPEND COMMAND_LINE --base "${BRIDGE_CPP_BASE}")
    endif()
    if(DEFINED BRIDGE_CPP_NAMESPACE)
        list(APPEND COMMAND_LINE --namespace ${BRIDGE_CPP_NAMESPACE})
    endif()

    # message(STATUS ${COMMAND_LINE})

    add_custom_command(
        OUTPUT "${BRIDGE_CPP_OUTPUT}"
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tool/LuaAutoBridgeTool/LuaAutoBridgeTool.py ${COMMAND_LINE}
        DEPENDS ${BRIDGE_CPP_FILES} "${CMAKE_SOURCE_DIR}/tool/LuaAutoBridgeTool/LuaAutoBridgeTool.py"
        COMMENT "Running auto bridge tool" VERBATIM )

    set_source_files_properties("${BRIDGE_CPP_OUTPUT}" PROPERTIES GENERATED TRUE)
endfunction()
