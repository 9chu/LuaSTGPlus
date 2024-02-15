### 生成版本号信息

find_package(Git)
add_custom_target(lstg.PlusV2Version
    ${CMAKE_COMMAND} -D SRC=${CMAKE_CURRENT_SOURCE_DIR}/Version.hpp.in
    -D DST=${CMAKE_CURRENT_BINARY_DIR}/Version.hpp
    -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/GenerateVersionHeader.cmake)
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/Version.hpp" PROPERTIES GENERATED TRUE)

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

set(LSTG_V2_SOURCES_GEN)
list(APPEND LSTG_V2_SOURCES_GEN "${CMAKE_CURRENT_BINARY_DIR}/Version.hpp")
list(APPEND LSTG_V2_SOURCES_GEN "${CMAKE_CURRENT_BINARY_DIR}/BuiltInModules.gen.cpp")
list(APPEND LSTG_V2_SOURCES_GEN "${CMAKE_CURRENT_BINARY_DIR}/ScriptObjectAttributes.gen.hpp")
list(APPEND LSTG_V2_SOURCES_GEN "${CMAKE_CURRENT_BINARY_DIR}/ScriptObjectAttributes.gen.cpp")

# 平台定制代码
if(LSTG_PLATFORM_WIN32)
    list(APPEND LSTG_V2_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/LuaSTGPlus2.rc")
endif()

# 目标
if(LSTG_PLATFORM_WIN32)
    add_executable(lstg.PlusV2 WIN32 ${LSTG_V2_SOURCES} ${LSTG_V2_SOURCES_GEN})
elseif(LSTG_PLATFORM_ANDROID)
    add_library(lstg.PlusV2 SHARED ${LSTG_V2_SOURCES} ${LSTG_V2_SOURCES_GEN})
else()
    add_executable(lstg.PlusV2 ${LSTG_V2_SOURCES} ${LSTG_V2_SOURCES_GEN})
endif()
target_include_directories(lstg.PlusV2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(lstg.PlusV2 PUBLIC
    lstg::Core
    sdl2::SDL2main
    sdl2::SDL2-static
    luacjson::cjson
    imgui::imgui
    implot::implot)
add_dependencies(lstg.PlusV2 lstg.PlusV2Version)
set_target_properties(lstg.PlusV2 PROPERTIES EXPORT_NAME PlusV2 OUTPUT_NAME LuaSTGPlus2)

#set(LSTG_V2_DEFS_PRIVATE)
#message("[LSTG-v2] Private compiler definitions: ${LSTG_V2_DEFS_PRIVATE}")
#target_compile_definitions(lstg.PlusV2 PRIVATE ${LSTG_V2_DEFS_PRIVATE})

# 平台特定链接选项
if(LSTG_PLATFORM_EMSCRIPTEN)
    set(LSTG_EMSCRIPTEN_LINK_OPTIONS
        "SHELL:-s FETCH=1"
        "SHELL:-s FETCH_SUPPORT_INDEXEDDB=0"
        "SHELL:-s MIN_WEBGL_VERSION=2"
        "SHELL:-s MAX_WEBGL_VERSION=3"
        "SHELL:-s GL_ENABLE_GET_PROC_ADDRESS=1"
        "SHELL:-s FULL_ES3=1"
        "SHELL:-s ENVIRONMENT=web"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s INITIAL_MEMORY=256mb")
    if(NOT LSTG_SHIPPING AND LSTG_ENABLE_ASSERTIONS)
        list(APPEND LSTG_EMSCRIPTEN_LINK_OPTIONS
            "SHELL:-s ASSERTIONS=1"
            "SHELL:-s SAFE_HEAP=1"
            "SHELL:-s DEMANGLE_SUPPORT=1")
    endif()
    message(STATUS "[LSTG] Emscripten link options: ${LSTG_EMSCRIPTEN_LINK_OPTIONS}")
    target_link_options(lstg.PlusV2 PRIVATE ${LSTG_EMSCRIPTEN_LINK_OPTIONS})
endif()

# 优化 IDE 展示
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${LSTG_V2_SOURCES})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX generated FILES ${LSTG_V2_SOURCES_GEN})

