/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
