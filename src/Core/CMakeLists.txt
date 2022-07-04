### 明确第三方库引用
set(LSTG_CORE_DEPS_PUBLIC fmt liblua-static glm::glm nlohmann_json)
set(LSTG_CORE_DEPS_PRIVATE spdlog SDL2-static imgui implot zlib-ng stb freetype harfbuzz icuuc harfbuzz harfbuzz-icu freetype IcuData ryu)

# 平台相关库
if(LSTG_PLATFORM_MACOS)
    list(APPEND LSTG_CORE_DEPS_PUBLIC "-framework Cocoa")
endif()
if(LSTG_PLATFORM_EMSCRIPTEN)
    list(APPEND LSTG_CORE_DEPS_PUBLIC GL)
endif()

# Diligent
get_supported_backends(DILIGENT_ENGINE_LIBRARIES "static")
list(APPEND DILIGENT_ENGINE_LIBRARIES
    Diligent-Common
    Diligent-GraphicsAccessories
    Diligent-GraphicsTools
    Diligent-TargetPlatform
    Diligent-PublicBuildSettings)
message("[LSTG] Diligent libraries: ${DILIGENT_ENGINE_LIBRARIES}")
list(APPEND LSTG_CORE_DEPS_PRIVATE ${DILIGENT_ENGINE_LIBRARIES})

message("[LSTG] Public libraries: ${LSTG_CORE_DEPS_PUBLIC}")
message("[LSTG] Private libraries: ${LSTG_CORE_DEPS_PRIVATE}")

### 目标
# Lua 自动封装代码
file(GLOB_RECURSE LSTG_CORE_RENDER_LUA_EFFECT_BUILDER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Subsystem/Render/detail/LuaEffectBuilder/*.hpp)
lstg_gen_auto_bridge_cpp(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/LuaEffectBuilder.gen.cpp"
    NAME "lstg::Subsystem::Render::detail::LuaEffectBuilder::InitModule"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/Subsystem/Render/detail/LuaEffectBuilder/"
    NAMESPACE "lstg::Subsystem::Render::detail::LuaEffectBuilder"
    FILES ${LSTG_CORE_RENDER_LUA_EFFECT_BUILDER_SOURCES})

# 跨平台源码
file(GLOB_RECURSE LSTG_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lstg/Core/*.hpp)
list(APPEND LSTG_CORE_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/LuaEffectBuilder.gen.cpp")

# 平台定制代码
if(LSTG_PLATFORM_MACOS)
    list(APPEND LSTG_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Subsystem/Render/detail/RenderDevice/OSX/GLView.mm)
    list(APPEND LSTG_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Subsystem/Render/detail/RenderDevice/OSX/MetalView.mm)
endif()

# 目标
add_library(LuaSTGPlusCore STATIC ${LSTG_CORE_SOURCES})
target_include_directories(LuaSTGPlusCore
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Subsystem/Render/detail/LuaEffectBuilder)
target_link_libraries(LuaSTGPlusCore PUBLIC ${LSTG_CORE_DEPS_PUBLIC} PRIVATE ${LSTG_CORE_DEPS_PRIVATE})

# 宏（公开的）
set(LSTG_CORE_DEFS_PUBLIC LSTG_APP_NAME="${LSTG_APP_NAME}")
if(LSTG_SHIPPING)  # Shipping/Development 开关
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_SHIPPING)
else()
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_DEVELOPMENT)
endif()
if(LSTG_PLATFORM_WIN32)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_WIN32)
endif()
if(LSTG_PLATFORM_LINUX)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_LINUX)
endif()
if(LSTG_PLATFORM_MACOS)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_MACOS)
endif()
if(LSTG_PLATFORM_EMSCRIPTEN)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_EMSCRIPTEN)
endif()
if(LSTG_PLATFORM_MACOS)  # Apple = MacOS | iOS
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_APPLE)
endif()
message("[LSTG] Public compiler definitions: ${LSTG_CORE_DEFS_PUBLIC}")
target_compile_definitions(LuaSTGPlusCore PUBLIC ${LSTG_CORE_DEFS_PUBLIC})
