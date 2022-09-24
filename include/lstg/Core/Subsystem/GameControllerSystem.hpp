/**
 * @file
 * @date 2022/8/24
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <map>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include "../Result.hpp"
#include "../Exception.hpp"
#include "ISubsystem.hpp"
#include "EventBusSystem.hpp"
#include "WindowSystem.hpp"
#include "VFS/IStream.hpp"
#include "GameController/ScanCodeMapping.hpp"

typedef struct _SDL_GameController SDL_GameController;

namespace lstg::Subsystem
{
    LSTG_DEFINE_EXCEPTION(GameControllerInitializeFailedException);

    namespace detail
    {
        struct GameControllerDeleter
        {
            void operator()(SDL_GameController* controller) noexcept;
        };

        using SDLGameControllerPtr = std::unique_ptr<SDL_GameController, GameControllerDeleter>;
    }
    
    /**
     * 游戏手柄输入系统
     */
    class GameControllerSystem :
        public ISubsystem
    {
    private:
        struct GameControllerInfo
        {
            detail::SDLGameControllerPtr Device;
            int32_t InstanceId = 0;
            std::string Guid;
            const char* Name = nullptr;
            GameController::ScanCodeMappingConfigPtr ScanCodeMapping;
        };

        static GameControllerInfo OpenController(int32_t index) noexcept;

    public:
        GameControllerSystem(SubsystemContainer& container);
        GameControllerSystem(const GameControllerSystem&) = delete;
        GameControllerSystem(GameControllerSystem&&)noexcept = delete;

    public:
        /**
         * 从流加载扫描码映射
         * @param stream 流
         * @return 是否成功
         */
        Result<void> LoadScanCodeMappings(VFS::IStream* stream) noexcept;

    public:  // ISubsystem
        void OnEvent(SubsystemEvent& event) noexcept override;

    private:
        void ApplyNewScanCodeMapping() noexcept;
        void ApplyNewScanCodeMappingOne(GameControllerInfo& info) noexcept;
        void HandleScanCodeMapping(GameController::ScanCode scanCode, GameController::ButtonTriggerMode mode, int state,
            uint32_t timestamp) noexcept;

    private:
        EventBusSystemPtr m_pEventBusSystem;
        uint32_t m_uMainWindowId = 0;

        std::map<int32_t, GameControllerInfo> m_stControllers;
        std::map<std::string, GameController::ScanCodeMappingConfigPtr, std::less<>> m_stScanCodeMappings;
        std::vector<bool> m_stScanCodeKeyStateMap;  // 追踪 ScanCode 按键状态
    };

    using GameControllerSystemPtr = std::shared_ptr<GameControllerSystem>;
}
