# 目标
file(GLOB_RECURSE LSTG_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lstg/Core/*.hpp)
add_library(LuaSTGPlusCore STATIC ${LSTG_CORE_SOURCES})
target_include_directories(LuaSTGPlusCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

## 第三方库

# FIXME: 暂时适配 PiotrSikora/zlib-ng 的改动
if (EMSCRIPTEN)
    set(ZLIB_LIBS zlib)
else()
    set(ZLIB_LIBS zlibstatic)
endif()

target_link_libraries(LuaSTGPlusCore
    PUBLIC fmt
    PRIVATE spdlog SDL2-static ${ZLIB_LIBS}
)
