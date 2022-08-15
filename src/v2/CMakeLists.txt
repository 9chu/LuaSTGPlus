### 目标
# Lua 自动封装代码
file(GLOB_RECURSE LSTG_V2_BRIDGE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lstg/v2/Bridge/*.hpp)
lstg_gen_auto_bridge_cpp(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/BuiltInModules.gen.cpp"
    NAME "lstg::v2::Bridge::InitBuiltInModule"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/../../include/"
    NAMESPACE "lstg::v2::Bridge"
    FILES ${LSTG_V2_BRIDGE_SOURCES})

# 完美哈希生成
lstg_gen_perfect_hasher(
    DECL "${CMAKE_CURRENT_SOURCE_DIR}/ScriptObjectAttributes.json"
    OUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ScriptObjectAttributes.gen.hpp"
    OUT_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/ScriptObjectAttributes.gen.cpp")

# 跨平台源码
file(GLOB_RECURSE LSTG_V2_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lstg/v2/*.hpp)
list(APPEND LSTG_V2_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/BuiltInModules.gen.cpp")
list(APPEND LSTG_V2_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/ScriptObjectAttributes.gen.cpp")

# 目标
if(WIN32)
    add_executable(LuaSTGPlus2 WIN32 ${LSTG_V2_SOURCES})
else()
    add_executable(LuaSTGPlus2 ${LSTG_V2_SOURCES})
endif()
target_include_directories(LuaSTGPlus2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(LuaSTGPlus2 PUBLIC LuaSTGPlusCore SDL2main cjson imgui implot)

# 平台特定链接选项
if(LSTG_PLATFORM_EMSCRIPTEN)
    set(LSTG_EMSCRIPTEN_LINK_OPTIONS
        "SHELL:-s ASYNCIFY"
        "SHELL:-s FETCH=1"
        "SHELL:-s FETCH_SUPPORT_INDEXEDDB=0"
        "SHELL:-s MIN_WEBGL_VERSION=2"
        "SHELL:-s MAX_WEBGL_VERSION=3"
        "SHELL:-s FULL_ES3=1"
        "SHELL:-s ENVIRONMENT=web"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s INITIAL_MEMORY=256mb")
    if(NOT LSTG_SHIPPING)
        list(APPEND LSTG_EMSCRIPTEN_LINK_OPTIONS
            "SHELL:-s ASSERTIONS=1"
            "SHELL:-s SAFE_HEAP=1"
            "SHELL:-s DEMANGLE_SUPPORT=1")
    endif()
    message(STATUS "[LSTG] Emscripten link options: ${LSTG_EMSCRIPTEN_LINK_OPTIONS}")
    target_link_options(LuaSTGPlus2 PRIVATE ${LSTG_EMSCRIPTEN_LINK_OPTIONS})
endif()
