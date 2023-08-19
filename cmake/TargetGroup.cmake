# 在某些 IDE 中折叠目标到单独的目录里（比如 Visual Studio）
function(_set_targets_into_ide_folder folder targets)
    foreach(target ${targets})
        #message(STATUS "check target - " ${target})
        if(TARGET ${target})
            #message(STATUS "fold " ${target} "-\>" ${folder})
            set_target_properties(${target} PROPERTIES FOLDER ${folder})
        endif()
    endforeach()
endfunction()
#
set(_sdl2_target_list
    sdl_headers_copy
    SDL2
    SDL2_sound-static
    SDL2main
    SDL2-static
    mojoal)
_set_targets_into_ide_folder(SDL2 "${_sdl2_target_list}")
#
set(_lua_target_list
    minilua
    buildvm
    liblua-shared
    liblua-static
    lua
    lua-static
    luabitop
    cjson)
_set_targets_into_ide_folder(lua "${_lua_target_list}")
#
set(_imgui_target_list
    imgui
    implot)
_set_targets_into_ide_folder(imgui "${_imgui_target_list}")
#
set(_compose_target_list
    freetype
    harfbuzz
    harfbuzz-icu)
_set_targets_into_ide_folder(compose "${_compose_target_list}")
#
set(_icu_target_list
        icuuc
        icuin
        icuio
        icutu
        IcuData)
set(_icu_tools
    gencnval
    gencfu
    makeconv
    genbrk
    gensprep
    gendict
    icupkg
    genrb
    pkgdata)
foreach(target ${_icu_tools})
    list(APPEND _icu_target_list ${target})
    list(APPEND _icu_target_list PrepareTool_${target})
endforeach()
_set_targets_into_ide_folder(compose/icu "${_icu_target_list}")
#
set(_others_target_list
    fmt
    ryu
    spdlog
    stb
    zlib-ng)
_set_targets_into_ide_folder(external "${_others_target_list}")
