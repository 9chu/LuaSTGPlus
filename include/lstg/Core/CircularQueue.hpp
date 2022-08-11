/**
* @file
* @date 2022/8/7
* @author 9chu
* 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <array>

namespace lstg
{
    /**
     * 定长环形队列
     * @tparam T 类型
     * @tparam Capacity 容量
     */
    template <typename T, size_t Capacity>
    class CircularQueue
    {
        static_assert(Capacity > 0);

        enum {
            kRealCapacity = Capacity + 1
        };

    public:
        CircularQueue() = default;
        
        const T& operator[](size_t idx) const noexcept
        {
            assert(idx < m_uCount);
            return m_stStorage[(idx + m_uFront) % kRealCapacity];
        }
        T& operator[](size_t idx) noexcept
        {
            assert(idx < m_uCount);
            return m_stStorage[(idx + m_uFront) % kRealCapacity];
        }
        
    public:
        /**
         * 队列是否为空
         */
        bool IsEmpty() const noexcept { return m_uFront == m_uTail; }

        /**
         * 队列是否已满
         */
        bool IsFull() const noexcept { return (m_uFront == (m_uTail + 1) % kRealCapacity); }

        /**
         * 获取当前元素个数
         */
        size_t GetSize() const noexcept { return m_uCount; }

        /**
         * 获取容量
         */
        size_t GetCapacity() const noexcept { return kRealCapacity - 1; }

        /**
         * 追加元素
         * @tparam P 类型
         * @param val 值
         * @return 是否成功
         */
        template <typename P>
        bool Push(P&& val) noexcept
        {
            if (IsFull())
            {
                return false;
            }
            else
            {
                m_stStorage[m_uTail] = std::forward<P>(val);
                m_uTail = (m_uTail + 1) % kRealCapacity;
                ++m_uCount;
                return true;
            }
        }

        /**
         * 弹出一个元素
         * @param out 元素值
         * @return 是否成功
         */
        bool Pop(T& out) noexcept
        {
            if (IsEmpty())
            {
                return false;
            }
            else
            {
                out = std::move(m_stStorage[m_uFront]);
                m_uFront = (m_uFront + 1) % kRealCapacity;
                --m_uCount;
                return true;
            }
        }

        /**
         * 获取队列头部元素
         */
        const T& Front() const noexcept
        {
            assert(!IsEmpty());
            return m_stStorage[m_uFront];
        }
        T& Front() noexcept
        {
            assert(!IsEmpty());
            return m_stStorage[m_uFront];
        }

        /**
         * 获取队列尾部元素
         */
        const T& Back() const noexcept
        {
            assert(!IsEmpty());
            if (m_uTail == 0)
                return m_stStorage[kRealCapacity - 1];
            else
                return m_stStorage[m_uTail - 1];
        }
        T& Back() noexcept
        {
            assert(!IsEmpty());
            if (m_uTail == 0)
                return m_stStorage[kRealCapacity - 1];
            else
                return m_stStorage[m_uTail - 1];
        }
        
    private:
        std::array<T, kRealCapacity> m_stStorage;
        size_t m_uFront = 0;
        size_t m_uTail = 0;
        size_t m_uCount = 0;
    };
}
