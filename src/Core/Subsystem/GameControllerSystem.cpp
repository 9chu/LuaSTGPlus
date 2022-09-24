/**
 * @file
 * @date 2022/8/24
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/GameControllerSystem.hpp>

#include <SDL.h>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Text/JsonHelper.hpp>
#include <lstg/Core/Subsystem/VFS/FileStream.hpp>
#include <lstg/Core/AppBase.hpp>  // for Cmdline

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

LSTG_DEF_LOG_CATEGORY(GameControllerSystem);

// <editor-fold desc="detail::GameControllerDeleter">

void Subsystem::detail::GameControllerDeleter::operator()(SDL_GameController* controller) noexcept
{
    ::SDL_GameControllerClose(controller);
}

// </editor-fold>

// <editor-fold desc="GameControllerSystem">

namespace
{
    const char kToHexTable[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
    };

    void AppendHex(std::string& out, uint8_t b)
    {
        out.push_back(kToHexTable[(b >> 4u) & 0xFu]);
        out.push_back(kToHexTable[b & 0xFu]);
    }
}

GameControllerSystem::GameControllerInfo GameControllerSystem::OpenController(int32_t index) noexcept
{
    GameControllerInfo controller;
    controller.Device.reset(::SDL_GameControllerOpen(index));
    if (!controller.Device)
    {
        LSTG_LOG_ERROR_CAT(GameControllerSystem, "Open game controller index {} fail: {}", index, SDL_GetError());
        return {};
    }

    try
    {
        auto joystick = ::SDL_GameControllerGetJoystick(controller.Device.get());
        assert(joystick);

        // 获取实例 ID
        controller.InstanceId = ::SDL_JoystickInstanceID(joystick);

        // 复制 GUID
        auto guid = ::SDL_JoystickGetGUID(joystick);
        controller.Guid.reserve(32);
        for (size_t i = 0; i < sizeof(guid.data); ++i)
            AppendHex(controller.Guid, guid.data[i]);

        // 复制名字
        controller.Name = ::SDL_JoystickName(joystick);
    }
    catch (...)
    {
        LSTG_LOG_ERROR_CAT(GameControllerSystem, "Memory not enough");
        return {};
    }
    return controller;
}

GameControllerSystem::GameControllerSystem(SubsystemContainer& container)
    : m_pEventBusSystem(container.Get<EventBusSystem>()),
    m_uMainWindowId(::SDL_GetWindowID(container.Get<WindowSystem>()->GetNativeHandle()))
{
    // 初始化 SDL 游戏手柄子系统
    int ev = ::SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    if (ev < 0)
    {
        LSTG_LOG_CRITICAL_CAT(GameControllerSystem, "Initialize SDL game controller subsystem fail, SDL_GetError: {}", SDL_GetError());
        LSTG_THROW(GameControllerInitializeFailedException, "Initialize SDL game controller subsystem fail, SDL_GetError: {}",
            SDL_GetError());
    }

    // 打开所有手柄
    auto joystickCount = ::SDL_NumJoysticks();
    for (auto i = 0; i < joystickCount; i++)
    {
        if (!::SDL_IsGameController(i))
            continue;

        auto controller = OpenController(i);
        if (!controller.Device)
            continue;

        auto id = controller.InstanceId;
        LSTG_LOG_INFO_CAT(GameControllerSystem, "Open game controller {}, id {}", controller.Name, id);

        auto it = m_stControllers.emplace(id, std::move(controller));
        static_cast<void>(it);
        assert(it.second);
    }

    // 从命令行配置默认映射
    auto cmdControllerToKeyConfig = AppBase::GetCmdline().GetOption<string_view>("controller-to-key-config", "");
    if (!cmdControllerToKeyConfig.empty())
    {
        LSTG_LOG_INFO_CAT(GameControllerSystem, "Loading controller to key configure from {}", cmdControllerToKeyConfig);

        auto fs = make_shared<VFS::FileStream>(cmdControllerToKeyConfig, VFS::FileAccessMode::Read, VFS::FileOpenFlags::None);
        LoadScanCodeMappings(fs.get()).ThrowIfError();
    }

    //
    m_stScanCodeKeyStateMap.resize(SDL_NUM_SCANCODES);
}

Result<void> GameControllerSystem::LoadScanCodeMappings(VFS::IStream* stream) noexcept
{
    // 读取整个配置数据
    string content;
    auto ret = VFS::ReadAll(content, stream);
    if (!ret)
        return ret.GetError();
    
    // 解析到 Json
    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(content);
    }
    catch (const std::exception& ex)
    {
        LSTG_LOG_ERROR_CAT(GameControllerSystem, "Parse error: {}", ex.what());
        return make_error_code(errc::invalid_argument);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }

    if (!json.is_array())
    {
        LSTG_LOG_ERROR_CAT(GameControllerSystem, "Array expected for mapping configure");
        return make_error_code(errc::invalid_argument);
    }

    // 从 Json 读取映射关系
    map<std::string, GameController::ScanCodeMappingConfigPtr, std::less<>> mapping;
    try
    {
        for (const auto& it : json)
        {
            GameController::ScanCodeMappingConfig config;
            config.ReadFrom(it);
            mapping[config.Guid] = make_shared<GameController::ScanCodeMappingConfig>(config);
        }
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
    m_stScanCodeMappings = std::move(mapping);

    // 重新设置映射关系
    ApplyNewScanCodeMapping();
    return {};
}

void GameControllerSystem::OnEvent(SubsystemEvent& event) noexcept
{
    if (event.GetEvent().index() == 0)
    {
        const SDL_Event* sdlEvent = std::get<const SDL_Event*>(event.GetEvent());
        assert(sdlEvent);

        switch (sdlEvent->type)
        {
            case SDL_CONTROLLERDEVICEADDED:  // 新设备插入
                {
                    auto controller = OpenController(sdlEvent->cdevice.which);  // 此时 which 是 index
                    if (!controller.Device)
                        break;

                    auto id = controller.InstanceId;
                    LSTG_LOG_INFO_CAT(GameControllerSystem, "New plugged game controller {}, id {}", controller.Name, id);

                    // 绑定映射
                    ApplyNewScanCodeMappingOne(controller);

                    try
                    {
                        // 我们发现即便是已经创建了设备，也会触发 ADDED 事件
                        m_stControllers[id] = std::move(controller);
                    }
                    catch (...)
                    {
                        LSTG_LOG_ERROR_CAT(GameControllerSystem, "Out of memory");
                    }
                }
                break;
            case SDL_CONTROLLERDEVICEREMOVED:  // 设备移除
                {
                    auto it = m_stControllers.find(sdlEvent->cdevice.which);  // 此时 which 是 InstanceID
                    if (m_stControllers.end() == it)
                    {
                        LSTG_LOG_WARN_CAT(GameControllerSystem, "Removed game controller not found, id {}", sdlEvent->cdevice.which);
                        break;
                    }

                    m_stControllers.erase(it);
                }
                break;
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERBUTTONDOWN:
                {
                    auto button = sdlEvent->cbutton.button;
                    if (button >= static_cast<size_t>(GameController::Buttons::Count_))
                        break;

                    // 找到设备
                    auto it = m_stControllers.find(sdlEvent->cbutton.which);
                    if (m_stControllers.end() == it)
                    {
                        LSTG_LOG_WARN_CAT(GameControllerSystem, "Unexpected joystick event, id {}", sdlEvent->cbutton.which);
                        break;
                    }

                    // 检查映射
                    auto mapping = it->second.ScanCodeMapping;
                    if (!mapping)
                        break;

                    auto map = mapping->ButtonMappings[button];
                    if (map.Mapping != 0)
                        HandleScanCodeMapping(map.Mapping, map.Mode, sdlEvent->cbutton.state, sdlEvent->cbutton.timestamp);
                }
                break;
            case SDL_CONTROLLERAXISMOTION:
                {
                    // 找到设备
                    auto it = m_stControllers.find(sdlEvent->caxis.which);
                    if (m_stControllers.end() == it)
                    {
                        LSTG_LOG_WARN_CAT(GameControllerSystem, "Unexpected joystick event, id {}", sdlEvent->caxis.which);
                        break;
                    }

                    // 检查映射
                    auto mapping = it->second.ScanCodeMapping;
                    if (!mapping)
                        break;

                    // 找到摇杆配置
                    glm::vec2 deadZone { -0.001, 0.001 };
                    GameController::ScanCode negativeMapping = 0, positiveMapping = 0;
                    switch (sdlEvent->caxis.axis)
                    {
                        case SDL_CONTROLLER_AXIS_LEFTX:
                            deadZone = mapping->AxisMappings[0].XDeadZone;
                            negativeMapping = mapping->AxisMappings[0].XNegativeMapping;
                            positiveMapping = mapping->AxisMappings[0].XPositiveMapping;
                            break;
                        case SDL_CONTROLLER_AXIS_LEFTY:
                            deadZone = mapping->AxisMappings[0].YDeadZone;
                            negativeMapping = mapping->AxisMappings[0].YNegativeMapping;
                            positiveMapping = mapping->AxisMappings[0].YPositiveMapping;
                            break;
                        case SDL_CONTROLLER_AXIS_RIGHTX:
                            deadZone = mapping->AxisMappings[1].XDeadZone;
                            negativeMapping = mapping->AxisMappings[1].XNegativeMapping;
                            positiveMapping = mapping->AxisMappings[1].XPositiveMapping;
                            break;
                        case SDL_CONTROLLER_AXIS_RIGHTY:
                            deadZone = mapping->AxisMappings[1].YDeadZone;
                            negativeMapping = mapping->AxisMappings[1].YNegativeMapping;
                            positiveMapping = mapping->AxisMappings[1].YPositiveMapping;
                            break;
                        default:
                            assert(false);
                            break;
                    }

                    // 如果在 DeadZone，则认为放开
                    float v = sdlEvent->caxis.value < 0 ?
                        -(static_cast<float>(sdlEvent->caxis.value) / static_cast<float>(numeric_limits<int16_t>::min())) :
                        (static_cast<float>(sdlEvent->caxis.value) / static_cast<float>(numeric_limits<int16_t>::max()));
                    if (deadZone.x < v && v < deadZone.y)
                    {
                        if (negativeMapping != 0 && negativeMapping < SDL_NUM_SCANCODES && m_stScanCodeKeyStateMap[negativeMapping])
                        {
                            HandleScanCodeMapping(negativeMapping, GameController::ButtonTriggerMode::Normal, SDL_RELEASED,
                                sdlEvent->caxis.timestamp);
                        }
                        else if (positiveMapping != 0 && positiveMapping < SDL_NUM_SCANCODES && m_stScanCodeKeyStateMap[positiveMapping])
                        {
                            HandleScanCodeMapping(positiveMapping, GameController::ButtonTriggerMode::Normal, SDL_RELEASED,
                                sdlEvent->caxis.timestamp);
                        }
                        break;
                    }

                    // 负向区域按下映射
                    if (v < 0.f && negativeMapping != 0)
                    {
                        HandleScanCodeMapping(negativeMapping, GameController::ButtonTriggerMode::Normal, SDL_PRESSED,
                            sdlEvent->caxis.timestamp);
                    }
                    if (v > 0.f && positiveMapping != 0)
                    {
                        HandleScanCodeMapping(positiveMapping, GameController::ButtonTriggerMode::Normal, SDL_PRESSED,
                            sdlEvent->caxis.timestamp);
                    }
                }
                break;
            default:
                break;
        }
    }
}

void GameControllerSystem::ApplyNewScanCodeMapping() noexcept
{
    LSTG_LOG_TRACE_CAT(GameControllerSystem, "Applying new scancode mapping, size={}", m_stScanCodeMappings.size());

    for (auto& controller : m_stControllers)
        ApplyNewScanCodeMappingOne(controller.second);
}

void GameControllerSystem::ApplyNewScanCodeMappingOne(GameControllerInfo& info) noexcept
{
    auto specificMapping = m_stScanCodeMappings.find(info.Guid);
    if (specificMapping != m_stScanCodeMappings.end())
    {
        info.ScanCodeMapping = specificMapping->second;
        return;
    }

    auto genericMapping = m_stScanCodeMappings.find("");
    if (genericMapping != m_stScanCodeMappings.end())
    {
        info.ScanCodeMapping = genericMapping->second;
        return;
    }

    info.ScanCodeMapping.reset();
}

void GameControllerSystem::HandleScanCodeMapping(GameController::ScanCode scanCode, GameController::ButtonTriggerMode mode, int state,
    uint32_t timestamp) noexcept
{
    assert(scanCode != 0);
    if (scanCode >= SDL_NUM_SCANCODES)
        return;

    if (mode == GameController::ButtonTriggerMode::Normal)
    {
        if (state == SDL_PRESSED && !m_stScanCodeKeyStateMap[scanCode])
        {
            m_stScanCodeKeyStateMap[scanCode] = true;

            // 发起按下按键消息
            SDL_Event fakeEvent;
            ::memset(&fakeEvent, 0, sizeof(fakeEvent));
            fakeEvent.type = SDL_KEYDOWN;
            fakeEvent.key.timestamp = timestamp;
            fakeEvent.key.state = SDL_PRESSED;
            fakeEvent.key.keysym.scancode = static_cast<SDL_Scancode>(scanCode);
            fakeEvent.key.repeat = 0;
            fakeEvent.key.windowID = m_uMainWindowId;
            m_pEventBusSystem->EmitEvent(fakeEvent);
        }
        else if (state == SDL_RELEASED && m_stScanCodeKeyStateMap[scanCode])
        {
            m_stScanCodeKeyStateMap[scanCode] = false;

            // 发起放开按键消息
            SDL_Event fakeEvent;
            ::memset(&fakeEvent, 0, sizeof(fakeEvent));
            fakeEvent.type = SDL_KEYUP;
            fakeEvent.key.timestamp = timestamp;
            fakeEvent.key.state = SDL_RELEASED;
            fakeEvent.key.keysym.scancode = static_cast<SDL_Scancode>(scanCode);
            fakeEvent.key.repeat = 0;
            fakeEvent.key.windowID = m_uMainWindowId;
            m_pEventBusSystem->EmitEvent(fakeEvent);
        }
    }
    else
    {
        assert(mode == GameController::ButtonTriggerMode::Switch);

        if (state == SDL_PRESSED)
        {
            if (m_stScanCodeKeyStateMap[scanCode])
            {
                m_stScanCodeKeyStateMap[scanCode] = false;

                // 此时放开按键
                SDL_Event fakeEvent;
                ::memset(&fakeEvent, 0, sizeof(fakeEvent));
                fakeEvent.type = SDL_KEYUP;
                fakeEvent.key.timestamp = timestamp;
                fakeEvent.key.state = SDL_RELEASED;
                fakeEvent.key.keysym.scancode = static_cast<SDL_Scancode>(scanCode);
                fakeEvent.key.repeat = 0;
                fakeEvent.key.windowID = m_uMainWindowId;
                m_pEventBusSystem->EmitEvent(fakeEvent);
            }
            else
            {
                m_stScanCodeKeyStateMap[scanCode] = true;

                // 此时按下按键
                SDL_Event fakeEvent;
                ::memset(&fakeEvent, 0, sizeof(fakeEvent));
                fakeEvent.type = SDL_KEYDOWN;
                fakeEvent.key.timestamp = timestamp;
                fakeEvent.key.state = SDL_PRESSED;
                fakeEvent.key.keysym.scancode = static_cast<SDL_Scancode>(scanCode);
                fakeEvent.key.repeat = 0;
                fakeEvent.key.windowID = m_uMainWindowId;
                m_pEventBusSystem->EmitEvent(fakeEvent);
            }
        }
    }
}

// </editor-fold>
