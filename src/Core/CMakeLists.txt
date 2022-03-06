# 目标
file(GLOB_RECURSE LSTG_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lstg/Core/*.hpp)
add_library(LuaSTGPlusCore STATIC ${LSTG_CORE_SOURCES})
target_include_directories(LuaSTGPlusCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

## 第三方库

# FIXME: 暂时适配 PiotrSikora/zlib-ng 的改动
if(EMSCRIPTEN)
    set(ZLIB_LIBS zlib)
    set(LUA_LIBS liblua_static)
    set(GL_LIBS GL)
else()
    set(ZLIB_LIBS zlibstatic)
    set(LUA_LIBS liblua-static)
    set(GL_LIBS)
endif()

target_link_libraries(LuaSTGPlusCore
    PUBLIC fmt bx ${LUA_LIBS} ${GL_LIBS}
    PRIVATE spdlog SDL2-static bgfx ${ZLIB_LIBS}
)
