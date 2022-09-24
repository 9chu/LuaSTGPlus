/**
* @file
* @date 2022/8/16
* @author 9chu
* 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
*/
#pragma once
#include <vector>
#include "Window.hpp"
#include "../../Result.hpp"

namespace lstg::Subsystem::DebugGUI
{
    /**
     * 进度窗口
     */
    class ProgressWindow :
        public Window
    {
    public:
        ProgressWindow();

    public:
        /**
         * 设置百分比
         */
        void SetPercent(float percent) noexcept { m_fPercent = percent; }

        /**
         * 设置说明文本
         */
        Result<void> SetHintText(std::string_view text) noexcept;

    protected:  // Window
        void OnPrepareWindow() noexcept override;
        void OnRender() noexcept override;

    private:
        float m_fPercent = 0;
        std::string m_stHintText;
    };
}



