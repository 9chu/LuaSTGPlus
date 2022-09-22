/**
 * @file
 * @date 2022/9/18
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <map>
#include <vector>
#include "../../Result.hpp"
#include "Window.hpp"

namespace lstg::Subsystem::DebugGUI
{
    /**
     * 混音器窗口
     */
    class MixerWindow :
        public Window
    {
    public:
        MixerWindow();

    protected:  // Window
        void OnPrepareWindow() noexcept override;
        void OnRender() noexcept override;

    private:
        Result<void> MakeKeysFromMapOptions(const std::map<std::string, int32_t>& options) noexcept;

    private:
        std::vector<const char*> m_stTmpOptions;
    };
}
