/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "AssetManagerModule.hpp"
#include "AudioModule.hpp"
#include "GameObjectModule.hpp"
#include "InputModule.hpp"
#include "MathModule.hpp"
#include "MiscModule.hpp"
#include "RenderModule.hpp"

namespace lstg::v2::Bridge
{
    void InitBuiltInModule(Subsystem::Script::LuaStack& stack);
}
