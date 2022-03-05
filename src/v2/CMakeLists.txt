# Lua 自动封装
file(GLOB_RECURSE LSTG_V2_BRIDGE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lstg/v2/Bridge/*.hpp)
lstg_gen_auto_bridge_cpp(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AutoBridge.gen.cpp"
    NAME "LuaModuleAutoBridge"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/../../include/"
    NAMESPACE "lstg::v2::Bridge"
    FILES ${LSTG_V2_BRIDGE_SOURCES})

# 目标
file(GLOB_RECURSE LSTG_V2_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lstg/v2/*.hpp)
list(APPEND LSTG_V2_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/AutoBridge.gen.cpp")

if(WIN32)
    add_executable(LuaSTGPlus2 WIN32 ${LSTG_V2_SOURCES})
else()
    add_executable(LuaSTGPlus2 ${LSTG_V2_SOURCES})
endif()
target_include_directories(LuaSTGPlus2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

# 第三方库
target_link_libraries(LuaSTGPlus2 PUBLIC LuaSTGPlusCore SDL2main)
if(EMSCRIPTEN)
    target_link_options(LuaSTGPlus2
        PRIVATE "SHELL:-s FETCH=1"
        PRIVATE "SHELL:-s FETCH_SUPPORT_INDEXEDDB=0"
        PRIVATE "SHELL:-s USE_PTHREADS=1"
        PRIVATE "SHELL:-s MIN_WEBGL_VERSION=2"
        PRIVATE "SHELL:-s MAX_WEBGL_VERSION=2"
        PRIVATE "SHELL:-s ENVIRONMENT=web,worker"
        PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        PRIVATE "SHELL:-s INITIAL_MEMORY=512mb"
        PRIVATE "SHELL:-s PROXY_TO_PTHREAD=1")
endif()
