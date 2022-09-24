/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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

#ifdef LSTG_DEVELOPMENT
            double EventDispatchTimeStack = 0.;
            double UpdateTimeStack = 0.;
            double RenderTimeStack = 0.;
            double AudioUpdateTime = 0.;
#endif
        };

        std::vector<FrameTime> m_stFrames;
    };
}
