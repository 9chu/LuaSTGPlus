/**
 * @file
 * @date 2022/8/25
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/GameController/ScanCodeMapping.hpp>

#include <SDL.h>
#include <map>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Text/JsonHelper.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::GameController;

LSTG_DEF_LOG_CATEGORY(ScanCodeMapping);

ScanCode Subsystem::GameController::ToScanCode(std::string_view descriptor) noexcept
{
    static const map<string, ScanCode, std::less<>> kKeyNameToScanCode = {
        { "backspace", SDL_SCANCODE_BACKSPACE },
        { "tab", SDL_SCANCODE_TAB },
        { "return", SDL_SCANCODE_RETURN },
        { "enter", SDL_SCANCODE_RETURN },
        { "shift", SDL_SCANCODE_LSHIFT },
        { "lshift", SDL_SCANCODE_LSHIFT },
        { "control", SDL_SCANCODE_LCTRL },
        { "lcontrol", SDL_SCANCODE_LCTRL },
        { "ctrl", SDL_SCANCODE_LCTRL },
        { "lctrl", SDL_SCANCODE_LCTRL },
        { "menu", SDL_SCANCODE_MENU },
        { "pause", SDL_SCANCODE_PAUSE },
        { "capslock", SDL_SCANCODE_CAPSLOCK },
        { "escape", SDL_SCANCODE_ESCAPE },
        { "space", SDL_SCANCODE_SPACE },
        { "pageup", SDL_SCANCODE_PAGEUP },
        { "pagedown", SDL_SCANCODE_PAGEDOWN },
        { "end", SDL_SCANCODE_END },
        { "home", SDL_SCANCODE_HOME },
        { "left", SDL_SCANCODE_LEFT },
        { "up", SDL_SCANCODE_UP },
        { "right", SDL_SCANCODE_RIGHT },
        { "down", SDL_SCANCODE_DOWN },
        { "select", SDL_SCANCODE_SELECT },
        { "execute", SDL_SCANCODE_EXECUTE },
        { "insert", SDL_SCANCODE_INSERT },
        { "delete", SDL_SCANCODE_DELETE },
        { "help", SDL_SCANCODE_HELP },
        { "0", SDL_SCANCODE_0 },
        { "1", SDL_SCANCODE_1 },
        { "2", SDL_SCANCODE_2 },
        { "3", SDL_SCANCODE_3 },
        { "4", SDL_SCANCODE_4 },
        { "5", SDL_SCANCODE_5 },
        { "6", SDL_SCANCODE_6 },
        { "7", SDL_SCANCODE_7 },
        { "8", SDL_SCANCODE_8 },
        { "9", SDL_SCANCODE_9 },
        { "a", SDL_SCANCODE_A },
        { "b", SDL_SCANCODE_B },
        { "c", SDL_SCANCODE_C },
        { "d", SDL_SCANCODE_D },
        { "e", SDL_SCANCODE_E },
        { "f", SDL_SCANCODE_F },
        { "g", SDL_SCANCODE_G },
        { "h", SDL_SCANCODE_H },
        { "i", SDL_SCANCODE_I },
        { "j", SDL_SCANCODE_J },
        { "k", SDL_SCANCODE_K },
        { "l", SDL_SCANCODE_L },
        { "m", SDL_SCANCODE_M },
        { "n", SDL_SCANCODE_N },
        { "o", SDL_SCANCODE_O },
        { "p", SDL_SCANCODE_P },
        { "q", SDL_SCANCODE_Q },
        { "r", SDL_SCANCODE_R },
        { "s", SDL_SCANCODE_S },
        { "t", SDL_SCANCODE_T },
        { "u", SDL_SCANCODE_U },
        { "v", SDL_SCANCODE_V },
        { "w", SDL_SCANCODE_W },
        { "x", SDL_SCANCODE_X },
        { "y", SDL_SCANCODE_Y },
        { "z", SDL_SCANCODE_Z },
        { "application", SDL_SCANCODE_APPLICATION },
        { "sleep", SDL_SCANCODE_SLEEP },
        { "num0", SDL_SCANCODE_KP_0 },
        { "num1", SDL_SCANCODE_KP_1 },
        { "num2", SDL_SCANCODE_KP_2 },
        { "num3", SDL_SCANCODE_KP_3 },
        { "num4", SDL_SCANCODE_KP_4 },
        { "num5", SDL_SCANCODE_KP_5 },
        { "num6", SDL_SCANCODE_KP_6 },
        { "num7", SDL_SCANCODE_KP_7 },
        { "num8", SDL_SCANCODE_KP_8 },
        { "num9", SDL_SCANCODE_KP_9 },
        { "nummul", SDL_SCANCODE_KP_MULTIPLY },
        { "numplus", SDL_SCANCODE_KP_PLUS },
        { "numminus", SDL_SCANCODE_KP_MINUS },
        { "numdiv", SDL_SCANCODE_KP_DIVIDE },
        { "f1", SDL_SCANCODE_F1 },
        { "f2", SDL_SCANCODE_F2 },
        { "f3", SDL_SCANCODE_F3 },
        { "f4", SDL_SCANCODE_F4 },
        { "f5", SDL_SCANCODE_F5 },
        { "f6", SDL_SCANCODE_F6 },
        { "f7", SDL_SCANCODE_F7 },
        { "f8", SDL_SCANCODE_F8 },
        { "f9", SDL_SCANCODE_F9 },
        { "f10", SDL_SCANCODE_F10 },
        { "f11", SDL_SCANCODE_F11 },
        { "f12", SDL_SCANCODE_F12 },
        { "f13", SDL_SCANCODE_F13 },
        { "f14", SDL_SCANCODE_F14 },
        { "f15", SDL_SCANCODE_F15 },
        { "f16", SDL_SCANCODE_F16 },
        { "f17", SDL_SCANCODE_F17 },
        { "f18", SDL_SCANCODE_F18 },
        { "f19", SDL_SCANCODE_F19 },
        { "f20", SDL_SCANCODE_F20 },
        { "f21", SDL_SCANCODE_F21 },
        { "f22", SDL_SCANCODE_F22 },
        { "f23", SDL_SCANCODE_F23 },
        { "f24", SDL_SCANCODE_F24 },
        { "f24", SDL_SCANCODE_F24 },
        { "numlock", SDL_SCANCODE_NUMLOCKCLEAR },
        { "scrolllock", SDL_SCANCODE_SCROLLLOCK },
        { "rcontrol", SDL_SCANCODE_RCTRL },
        { "rctrl", SDL_SCANCODE_RCTRL },
        { "rshift", SDL_SCANCODE_RSHIFT },
        { ";", SDL_SCANCODE_SEMICOLON },
        { "=", SDL_SCANCODE_EQUALS },
        { ",", SDL_SCANCODE_COMMA },
        { ".", SDL_SCANCODE_PERIOD },
        { "/", SDL_SCANCODE_SLASH },
        { "`", SDL_SCANCODE_GRAVE },
        { "[", SDL_SCANCODE_LEFTBRACKET },
        { "\\", SDL_SCANCODE_BACKSLASH },
        { "]", SDL_SCANCODE_RIGHTBRACKET },
        { "'", SDL_SCANCODE_APOSTROPHE },
        { "-", SDL_SCANCODE_MINUS },
    };

    auto it = kKeyNameToScanCode.find(descriptor);
    if (it == kKeyNameToScanCode.end())
        return 0;
    return it->second;
}

// <editor-fold desc="AxisMapping">

namespace
{
    std::optional<ScanCode> ReadScanCode(const nlohmann::json& json) noexcept
    {
        if (json.is_number())
        {
            return static_cast<ScanCode>(json.get<int32_t>());
        }
        else if (json.is_string())
        {
            auto ret = ToScanCode(json.get_ref<const string&>());
            if (ret == 0)
                LSTG_LOG_WARN_CAT(ScanCodeMapping, "Unknown scancode '{}'", json.get_ref<const string&>());
            return ret;
        }
        return {};
    }
}

void AxisMapping::ReadFrom(const nlohmann::json& json) noexcept
{
    XDeadZone = {-0.001, 0.001};
    YDeadZone = {-0.001, 0.001};
    XNegativeMapping = 0;
    XPositiveMapping = 0;
    YNegativeMapping = 0;
    YPositiveMapping = 0;

    if (json.is_object())
    {
#define READ_DEADZONE(FIELD_NAME, VALUE_OUT) \
        do { \
            if (json.contains(FIELD_NAME)) \
            { \
                auto v = json[FIELD_NAME]; \
                if (v.is_array() && v.size() >= 2 && v[0].is_number() && v[1].is_number()) \
                { \
                    VALUE_OUT.x = v[0].get<float>(); \
                    VALUE_OUT.y = v[1].get<float>(); \
                } \
                else \
                { \
                    LSTG_LOG_WARN_CAT(ScanCodeMapping, "Unexpected '" #FIELD_NAME "'"); \
                } \
            } \
        } while (false)

        READ_DEADZONE("xDeadZone", XDeadZone);
        READ_DEADZONE("yDeadZone", YDeadZone);

#undef READ_DEADZONE

#define READ_MAPPING(FIELD_NAME, VALUE_OUT) \
        do { \
            if (json.contains(FIELD_NAME)) \
            { \
                auto v = ReadScanCode(json[FIELD_NAME]); \
                if (!v) \
                    LSTG_LOG_WARN_CAT(ScanCodeMapping, "Unexpected '" FIELD_NAME "' type '{}'", json[FIELD_NAME].type_name()); \
                else \
                    VALUE_OUT = *v; \
            } \
        } while (false)

        READ_MAPPING("xNegativeMapping", XNegativeMapping);
        READ_MAPPING("xPositiveMapping", XPositiveMapping);
        READ_MAPPING("yNegativeMapping", YNegativeMapping);
        READ_MAPPING("yPositiveMapping", YPositiveMapping);

#undef READ_MAPPING
    }
}

// </editor-fold>

// <editor-fold desc="ButtonMapping">

void ButtonMapping::ReadFrom(const nlohmann::json& json) noexcept
{
    Mapping = 0;
    Mode = ButtonTriggerMode::Normal;

    auto scanCode = ReadScanCode(json);
    if (scanCode)
    {
        Mapping = *scanCode;
    }
    else if (json.is_object())
    {
        if (json.contains("mapping"))
        {
            auto ref = json["mapping"];
            scanCode = ReadScanCode(ref);
            if (scanCode)
            {
                Mapping = *scanCode;
            }
            else
            {
                LSTG_LOG_WARN_CAT(ScanCodeMapping, "Unexpected 'mapping' type '{}'", ref.type_name());
            }
        }

        if (json.contains("mode"))
        {
            auto ref = json["mode"];
            if (ref.is_number())
            {
                Mode = static_cast<ButtonTriggerMode>(ref.get<int32_t>());
            }
            else if (ref.is_string())
            {
                const auto& m = ref.get_ref<const string&>();
                if (m == "normal")
                    Mode = ButtonTriggerMode::Normal;
                else if (m == "switch")
                    Mode = ButtonTriggerMode::Switch;
                else
                    LSTG_LOG_WARN_CAT(ScanCodeMapping, "Unexpected 'mode' value '{}'", m);
            }
            else
            {
                LSTG_LOG_WARN_CAT(ScanCodeMapping, "Unexpected 'mode' type '{}'", ref.type_name());
            }
        }
    }
    else
    {
        LSTG_LOG_WARN_CAT(ScanCodeMapping, "Unexpected json value type '{}'", json.type_name());
    }
}

// </editor-fold>

// <editor-fold desc="ScanCodeMappingConfig">

void ScanCodeMappingConfig::ReadFrom(const nlohmann::json& json) noexcept
{
    // 读取 GUID
    Guid = Text::JsonHelper::ReadValue<string>(json, "/guid", "");

    // 读取按钮映射
    for (int i = 0; i < static_cast<int>(Buttons::Count_); ++i)
    {
        char buttonKey[64];
        sprintf(buttonKey, "button%s", ToString(static_cast<Buttons>(i)));

        if (json.is_object() && json.contains(buttonKey))
            ButtonMappings[i].ReadFrom(json[buttonKey]);
        else
            ButtonMappings[i] = {};
    }

    // 读取轴映射
    for (int i = 0; i < static_cast<int>(std::extent_v<decltype(AxisMappings)>); ++i)
    {
        char axisKey[64];
        sprintf(axisKey, "axis%d", i);

        if (json.is_object() && json.contains(axisKey))
            AxisMappings[i].ReadFrom(json[axisKey]);
        else
            AxisMappings[i] = {};
    }
}

// </editor-fold>
