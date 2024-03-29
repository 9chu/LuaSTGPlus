/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <lstg/Core/Flag.hpp>
#include "ISubsystem.hpp"

namespace lstg::Subsystem
{
    namespace detail
    {
        template <typename T>
        inline size_t GetUniqueSubsystemId()
        {
            static_assert(std::is_base_of_v<ISubsystem, T>, "Invalid type");

            static const int kUniqueId = 0;
            return reinterpret_cast<size_t>(&kUniqueId);
        }
    }

    LSTG_FLAG_BEGIN(SubsystemRegisterFlags)
        Default = 0,
        NoUpdate = 1,
        NoRender = 2,
        NoEvent = 4,
    LSTG_FLAG_END(SubsystemRegisterFlags)

    /**
     * 子系统容器
     * 提供简单的 IOC 构造机制
     */
    class SubsystemContainer
    {
    public:
        /**
         * 注册子系统
         * @tparam T 子系统类型
         * @param name 子系统名称
         * @param priority 优先级，越小优先级越高
         * @return 是否成功
         */
        template <typename T>
        bool Register(std::string name, int priority, SubsystemRegisterFlags flags = SubsystemRegisterFlags::Default)
        {
            // 构造 Storage
            auto storage = std::make_shared<SubsystemStorage>();
            storage->Flags = flags;
            storage->Priority = priority;
            storage->UniqueId = detail::GetUniqueSubsystemId<T>();
            storage->Name = std::move(name);
            storage->Constructor = [](SubsystemContainer& container) {
                return std::make_shared<T>(container);
            };
            return Register(storage);
        }

        /**
         * 获取子系统指针
         * @tparam T 子系统类型
         * @return 指针
         */
        template <typename T>
        std::shared_ptr<T> Get()
        {
            return std::static_pointer_cast<T>(FindById(detail::GetUniqueSubsystemId<T>()));
        }

        /**
         * 获取子系统指针
         * @tparam T 子系统类型
         * @param name 子系统名称
         * @return 指针
         */
        template <typename T = ISubsystem>
        std::shared_ptr<T> Get(std::string_view name)
        {
            return std::static_pointer_cast<T>(FindByName(name));
        }

        /**
         * 构造所有未初始化的子系统
         */
        void ConstructAll();

        /**
         * 更新所有子系统
         * @param elapsedTime 流逝时间
         */
        void Update(double elapsedTime) noexcept;

        /**
         * 用户渲染操作之前
         * @param elapsedTime 流逝时间
         */
        void BeforeRender(double elapsedTime) noexcept;

        /**
         * 用户渲染操作之后
         * @param elapsedTime 流逝时间
         */
        void AfterRender(double elapsedTime) noexcept;

        /**
         * 冒泡事件
         * @param event 事件
         */
        void BubbleEvent(SubsystemEvent& event) noexcept;

    private:
        enum class SubsystemStatus
        {
            NotInit = 0,
            Initializing = 1,
            Ready = 2,
        };

        struct SubsystemStorage
        {
            SubsystemStatus Status = SubsystemStatus::NotInit;
            SubsystemRegisterFlags Flags = SubsystemRegisterFlags::Default;
            int Priority = 0;
            size_t UniqueId = 0;
            std::string Name;
            SubsystemPtr Instance;
            std::function<SubsystemPtr(SubsystemContainer&)> Constructor;
        };

        using SubsystemStoragePtr = std::shared_ptr<SubsystemStorage>;

        void Construct(const SubsystemStoragePtr& storage);
        bool Register(SubsystemStoragePtr storage);
        SubsystemPtr FindByName(std::string_view name);
        SubsystemPtr FindById(size_t id);

    private:
        std::map<std::string, SubsystemStoragePtr, std::less<>> m_stSubsystems;
        std::unordered_map<size_t, SubsystemStoragePtr> m_stSubsystemById;
        std::multimap<int, SubsystemStoragePtr> m_stSubsystemUpdateChain;
        std::multimap<int, SubsystemStoragePtr> m_stSubsystemRenderChain;
        std::multimap<int, SubsystemStoragePtr> m_stSubsystemEventChain;
    };
}
