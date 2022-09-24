/**
 * @file
 * @date 2022/9/18
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
