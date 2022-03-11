/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <vector>
#include "Window.hpp"

namespace lstg::Subsystem::DebugGUI
{
    /**
     * 帧时间监控窗口
     */
    class FrameTimeMonitor :
        public Window
    {
    public:
        FrameTimeMonitor();

    protected:  // 需要实现
        virtual void OnPrepareWindow() noexcept override;
        virtual void OnUpdate(double elapsedTime) noexcept override;
        virtual void OnRender() noexcept override;

    private:
        struct FrameTime
        {
            double Total = 0.;
            double EventDispatchTimeStack = 0.;
            double UpdateTimeStack = 0.;
            double RenderTimeStack = 0.;
        };

        std::vector<FrameTime> m_stFrames;
    };
}
