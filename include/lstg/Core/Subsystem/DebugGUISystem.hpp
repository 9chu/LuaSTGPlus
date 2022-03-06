/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "RenderSystem.hpp"
#include "WindowSystem.hpp"

namespace lstg::Subsystem
{
    class DebugGUISystem :
        public ISubsystem
    {
    public:
        DebugGUISystem(SubsystemContainer& container);

    private:
        std::shared_ptr<WindowSystem> m_pWindowSystem;
        std::shared_ptr<RenderSystem> m_pRenderSystem;
    };
}
