# 目标
file(GLOB_RECURSE LSTG_GAME_APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lstg/App/*.hpp)

if(WIN32)
    add_executable(LuaSTGPlus WIN32 ${LSTG_GAME_APP_SOURCES})
else()
    add_executable(LuaSTGPlus ${LSTG_GAME_APP_SOURCES})
endif()
target_include_directories(LuaSTGPlus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

# 第三方库
set(LSTG_APP_DEPS LuaSTGPlusCore SDL2-static SDL2main)
if(EMSCRIPTEN)
    list(APPEND LSTG_APP_DEPS GL)
endif()

target_link_libraries(LuaSTGPlus PUBLIC ${LSTG_APP_DEPS})
if(EMSCRIPTEN)
    target_link_options(LuaSTGPlus
        PRIVATE "SHELL:-s MIN_WEBGL_VERSION=2"
        PRIVATE "SHELL:-s MAX_WEBGL_VERSION=2"
        PRIVATE "SHELL:-s ENVIRONMENT=web"
        PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
endif()
