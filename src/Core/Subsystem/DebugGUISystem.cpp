/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/DebugGUISystem.hpp>

#include <lstg/Core/Subsystem/SubsystemContainer.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

DebugGUISystem::DebugGUISystem(SubsystemContainer& container)
    : m_pWindowSystem(container.Get<WindowSystem>()), m_pRenderSystem(container.Get<RenderSystem>())
{
    assert(m_pWindowSystem && m_pRenderSystem);

}
