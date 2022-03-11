/**
 * @file
 * @date 2022/3/10
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once

namespace lstg::Subsystem::Render::detail::Emscripten
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
