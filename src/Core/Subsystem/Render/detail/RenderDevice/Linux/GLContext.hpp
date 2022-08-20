/**
 * @file
 * @date 2022/8/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>

namespace lstg::Subsystem::Render::detail::RenderDevice::Linux
{
    /**
     * GL 上下文
     */
    class GLContext
    {
    public:
        GLContext(/* _XDisplay */void* display, uint32_t windowId);
        ~GLContext();

    private:
        void* m_pDisplay = nullptr;
    };
}
