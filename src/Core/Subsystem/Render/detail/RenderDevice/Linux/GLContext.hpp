/**
 * @file
 * @date 2022/8/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
