### 明确第三方库引用

if(LSTG_PLATFORM_LINUX)
    find_package(X11)
    if(X11_FOUND)
        if(NOT X11_xcb_FOUND)
            message(FATAL_ERROR "X11-xcb is required to build this program")
        endif()
        set(LSTG_X11_ENABLE 1)
    endif()
endif()

set(LSTG_CORE_DEPS_PUBLIC
    fmt::fmt
    lstg::liblua-static
    glm::glm
    nlohmann_json::nlohmann_json)
set(LSTG_CORE_DEPS_PRIVATE
    spdlog::spdlog
    sdl2::SDL2-static
    imgui::imgui
    implot::implot
    zlib::zlib
    stb::stb
    freetype::freetype
    icu.libsicuuc
    harfbuzz::harfbuzz-icu
    lstg::IcuData
    double-conversion::double-conversion
    sdl2_sound::SDL2_sound-static)
if(LSTG_X11_ENABLE)
    list(APPEND LSTG_CORE_DEPS_PRIVATE X11::xcb X11::X11_xcb)
endif()
if(NOT LSTG_PLATFORM_EMSCRIPTEN)
    list(APPEND LSTG_CORE_DEPS_PRIVATE mojoal::mojoal)
else()
    list(APPEND LSTG_CORE_DEPS_PRIVATE luabitop::luabitop-static brotli::brotlicommon)
endif()

# 平台相关库
if(LSTG_PLATFORM_MACOS)
    list(APPEND LSTG_CORE_DEPS_PUBLIC "-framework Cocoa")
endif()
if(LSTG_PLATFORM_EMSCRIPTEN)
    list(APPEND LSTG_CORE_DEPS_PUBLIC GL openal)
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

set(LSTG_CORE_SOURCES_GEN)
list(APPEND LSTG_CORE_SOURCES_GEN "${CMAKE_CURRENT_BINARY_DIR}/LuaEffectBuilder.gen.cpp")

# 平台定制代码
if(LSTG_PLATFORM_MACOS)
    list(APPEND LSTG_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Subsystem/Render/detail/RenderDevice/OSX/GLView.mm)
    list(APPEND LSTG_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Subsystem/Render/detail/RenderDevice/OSX/MetalView.mm)
endif()

# 目标
add_library(lstg.Core STATIC ${LSTG_CORE_SOURCES} ${LSTG_CORE_SOURCES_GEN})
add_library(lstg::Core ALIAS lstg.Core)
target_include_directories(lstg.Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Subsystem/Render/detail/LuaEffectBuilder)
target_link_libraries(lstg.Core PUBLIC ${LSTG_CORE_DEPS_PUBLIC} PRIVATE ${LSTG_CORE_DEPS_PRIVATE})
set_target_properties(lstg.Core PROPERTIES EXPORT_NAME Core OUTPUT_NAME LuaSTGPlusCore)

# 宏（公开的）
set(LSTG_CORE_DEFS_PUBLIC LSTG_APP_NAME="${LSTG_APP_NAME}")
if(LSTG_SHIPPING)  # Shipping/Development 开关
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_SHIPPING)
else()
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_DEVELOPMENT)
endif()
if(LSTG_PARSE_CMDLINE)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PARSE_CMDLINE)
endif()
if(LSTG_DISABLE_HOT_RELOAD)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_ASSET_HOT_RELOAD=0)
endif()
if(LSTG_PLATFORM_WIN32)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_WIN32)
endif()
if(LSTG_PLATFORM_LINUX)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_LINUX)
    if(LSTG_X11_ENABLE)
        list(APPEND LSTG_CORE_DEFS_PRIVATE LSTG_X11_ENABLE)
    endif()
endif()
if(LSTG_PLATFORM_MACOS)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_MACOS)
endif()
if(LSTG_PLATFORM_EMSCRIPTEN)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_EMSCRIPTEN)
endif()
if(LSTG_PLATFORM_ANDROID)
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_ANDROID)
endif()
if(LSTG_PLATFORM_MACOS)  # Apple = MacOS | iOS
    list(APPEND LSTG_CORE_DEFS_PUBLIC LSTG_PLATFORM_APPLE)
endif()
message("[LSTG] Public compiler definitions: ${LSTG_CORE_DEFS_PUBLIC}")
target_compile_definitions(lstg.Core PUBLIC ${LSTG_CORE_DEFS_PUBLIC})
target_compile_definitions(lstg.Core PRIVATE ${LSTG_CORE_DEFS_PRIVATE})

# 优化 IDE 展示
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${LSTG_CORE_SOURCES})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX generated FILES ${LSTG_CORE_SOURCES_GEN})
