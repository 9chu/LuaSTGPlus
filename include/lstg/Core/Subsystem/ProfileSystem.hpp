/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <map>
#include <string>
#include <string_view>
#include <chrono>
#include "../Result.hpp"
#include "ISubsystem.hpp"

namespace lstg::Subsystem
{
    /**
     * 性能计数器类型
     */
    enum class PerformanceCounterTypes
    {
        RealTime = 0,
        PerFrame = 1,
    };
    
    /**
     * 性能记录 / 剖析系统
     */
    class ProfileSystem :
        public ISubsystem
    {
    public:
        /**
         * 获取全局实例
         */
        static ProfileSystem& GetInstance() noexcept;

    public:
        ProfileSystem(SubsystemContainer& container);

        ProfileSystem(const ProfileSystem&) = delete;
        ProfileSystem(ProfileSystem&&) = delete;

        ~ProfileSystem() override;

    public:
        /**
         * 获取性能计数器
         * 对于帧计数器，总是获取上一帧的数据。
         * @param type 类型
         * @param name 名称
         * @return 值
         */
        double GetPerformanceCounter(PerformanceCounterTypes type, std::string_view name) const noexcept;

        /**
         * 设置性能计数器
         * 对于帧计数器，总是设置当前帧的数据。
         * @param type 类型
         * @param name 名称
         * @param value 值
         * @return 是否成功
         */
        Result<void> SetPerformanceCounter(PerformanceCounterTypes type, std::string_view name, double value) noexcept;

        /**
         * 增加性能计数器值
         * 对于帧计数器，总是设置当前帧的数据。
         * @param type 类型
         * @param name 名称
         * @param add 增加的值
         * @return 是否成功
         */
        Result<void> IncrementPerformanceCounter(PerformanceCounterTypes type, std::string_view name, double add) noexcept;

        /**
         * 获取上一帧的时间
         */
        double GetLastFrameElapsedTime() const noexcept { return m_ullLastFrameElapsedTime; }

        /**
         * 通知开始新的一帧
         */
        void NewFrame() noexcept;

    private:
        std::chrono::steady_clock::time_point m_ullLastFrameTime;

        double m_ullLastFrameElapsedTime;  // seconds
        std::map<std::string, double, std::less<>> m_stRealTimeCounter;
        std::map<std::string, double, std::less<>> m_stPerFrameCounter;
        std::map<std::string, double, std::less<>> m_stLastFramePerFrameCounter;
    };

    namespace detail
    {
        template <PerformanceCounterTypes Type, bool Increment>
        class RunningTimeProfileHelper
        {
        public:
            RunningTimeProfileHelper(const char* name) noexcept
                : m_pName(name), m_stStart(std::chrono::steady_clock::now())
            {
            }

            ~RunningTimeProfileHelper() noexcept
            {
                auto end = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_stStart).count() / 1000000000.;

                if constexpr (Increment)
                    ProfileSystem::GetInstance().IncrementPerformanceCounter(Type, m_pName, elapsed);
                else
                    ProfileSystem::GetInstance().SetPerformanceCounter(Type, m_pName, elapsed);
            }

        private:
            const char* m_pName;
            std::chrono::steady_clock::time_point m_stStart;
        };
    }
}

#define LSTG_PER_FRAME_PROFILE(NAME) \
    lstg::Subsystem::detail::RunningTimeProfileHelper<lstg::Subsystem::PerformanceCounterTypes::PerFrame, true> NAME##Profiler_{#NAME}
