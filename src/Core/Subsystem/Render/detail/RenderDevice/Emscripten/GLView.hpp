/**
 * @file
 * @date 2022/3/10
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once

namespace lstg::Subsystem::Render::detail::RenderDevice::Emscripten
{
    /**
     * Emscripten 下 WebGL 视图
     * 用于创建 GLContext
     */
    class GLView
    {
    public:
        GLView(const char* canvasId);
        ~GLView();

    public:
        const char* GetView() const noexcept { return m_pCanvasId; }
        void Present() noexcept;

    private:
        const char* m_pCanvasId;
        int m_hContextHandle = 0;
    };
}
