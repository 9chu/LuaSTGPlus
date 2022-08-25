/**
 * @file
 * @date 2022/8/25
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string>
#include <string_view>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include "Buttons.hpp"

namespace lstg::Subsystem::GameController
{
    using ScanCode = int32_t;

    /**
     * 键描述字符串转键代码
     * @param descriptor 描述
     * @return 键代码，如果不存在则返回 0
     */
    ScanCode ToScanCode(std::string_view descriptor) noexcept;

    /**
     * 按钮切换模式
     */
    enum class ButtonTriggerMode
    {
        Normal,  ///< @brief 常规模式
        Switch,  ///< @brief 开关模式
    };

    /**
     * 轴映射
     */
    struct AxisMapping
    {
        glm::vec2 XDeadZone = {-0.001, 0.001};  // (lowerbound, upperbound)
        glm::vec2 YDeadZone = {-0.001, 0.001};
        ScanCode XNegativeMapping = 0;
        ScanCode XPositiveMapping = 0;
        ScanCode YNegativeMapping = 0;
        ScanCode YPositiveMapping = 0;

        /**
         * 从 JSON 加载
         */
        void ReadFrom(const nlohmann::json& json) noexcept;
    };

    /**
     * 按键映射
     */
    struct ButtonMapping
    {
        ScanCode Mapping = 0;
        ButtonTriggerMode Mode = ButtonTriggerMode::Normal;

        /**
         * 从 JSON 加载
         */
        void ReadFrom(const nlohmann::json& json) noexcept;
    };

    /**
     * 控制器到键盘按键映射配置
     */
    struct ScanCodeMappingConfig
    {
        std::string Guid;  // empty for all controllers
        ButtonMapping ButtonMappings[static_cast<size_t>(Buttons::Count_)];
        AxisMapping AxisMappings[2];  // left, right

        /**
         * 从 JSON 加载
         */
        void ReadFrom(const nlohmann::json& json) noexcept;
    };

    using ScanCodeMappingConfigPtr = std::shared_ptr<ScanCodeMappingConfig>;
}
