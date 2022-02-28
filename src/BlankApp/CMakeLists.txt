# 目标
file(GLOB_RECURSE LSTG_BLANK_APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

if(WIN32)
    add_executable(LuaSTGBlankApp WIN32 ${LSTG_BLANK_APP_SOURCES})
else()
    add_executable(LuaSTGBlankApp ${LSTG_BLANK_APP_SOURCES})
endif()
target_include_directories(LuaSTGBlankApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

# 第三方库
set(LSTG_APP_DEPS LuaSTGPlusCore SDL2-static SDL2main)
if(EMSCRIPTEN)
    list(APPEND LSTG_APP_DEPS GL)
endif()

target_link_libraries(LuaSTGBlankApp PUBLIC ${LSTG_APP_DEPS})
if(EMSCRIPTEN)
    target_link_options(LuaSTGBlankApp
        PRIVATE "SHELL:-s MIN_WEBGL_VERSION=2"
        PRIVATE "SHELL:-s MAX_WEBGL_VERSION=2"
        PRIVATE "SHELL:-s ENVIRONMENT=web"
        PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
endif()
