/**
 * @file
 * @date 2022/4/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <memory>
#include <utility>

namespace lstg::Subsystem::Render::Drawing2D
{
    template <typename T>
    class FreeList;

    namespace detail
    {
        template <typename T>
        struct FreeListRecycleHolder
        {
            FreeListRecycleHolder* Next = nullptr;
            T Object;

            FreeListRecycleHolder() = default;
        };

        template <typename T>
        struct FreeListHandle
        {
            FreeList<T>* FreeList = nullptr;
            FreeListRecycleHolder<T>* Holder = nullptr;

            FreeListHandle(std::nullptr_t = nullptr) noexcept
            {}

            FreeListHandle(Drawing2D::FreeList<T>* freeList, FreeListRecycleHolder<T>* holder)
                : FreeList(freeList), Holder(holder) {}

            FreeListHandle(const FreeListHandle& org) noexcept
                : FreeList(org.FreeList), Holder(org.Holder) {}

            FreeListHandle& operator=(const FreeListHandle& rhs) noexcept
            {
                if (this == &rhs)
                    return *this;
                FreeList = rhs.FreeList;
                Holder = rhs.Holder;
                return *this;
            }

            const T& operator*() const noexcept
            {
                return const_cast<FreeListHandle*>(this)->operator*();
            }
            T& operator*() noexcept
            {
                return Holder ? Holder->Object : (assert(false), *static_cast<T*>(nullptr));
            }

            const T* operator->() const noexcept
            {
                return &this->operator*();
            }
            T* operator->() noexcept
            {
                return &this->operator*();
            }

            explicit operator bool() noexcept
            {
                if (Holder)
                {
                    assert(FreeList);
                    return true;
                }
                return false;
            }
        };

        template <typename T>
        bool operator==(const FreeListHandle<T>& lhs, const FreeListHandle<T>& rhs) noexcept
        {
            return lhs.FreeList == rhs.FreeList && lhs.Holder == rhs.Holder;
        }

        template <typename T>
        bool operator!=(const FreeListHandle<T>& lhs, const FreeListHandle<T>& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        template <typename T>
        struct FreeListRecycleDeleter
        {
            using pointer = FreeListHandle<T>;

            void operator()(pointer p) const noexcept
            {
                if (p.Holder)
                {
                    assert(p.FreeList);
                    p.FreeList->Recycle(p.Holder);
                }
            }
        };
    }

    template <typename T>
    using FreeListPtr = std::unique_ptr<void, detail::FreeListRecycleDeleter<T>>;

    /**
     * 空闲列表
     */
    template <typename T>
    class FreeList
    {
        using Holder = detail::FreeListRecycleHolder<T>;

        friend struct detail::FreeListRecycleDeleter<T>;

    public:
        FreeList() = default;

        FreeList(const FreeList&) = delete;
        FreeList(FreeList&&) = delete;

        ~FreeList()
        {
            Free();
        }

    public:
        /**
         * 分配一个对象
         */
        FreeListPtr<T> Alloc()
        {
            auto holder = Awake();
            FreeListPtr<T> ret;
            ret.reset(detail::FreeListHandle<T> { this, holder });
            return ret;
        }

        /**
         * 释放空闲对象的内存
         */
        void Free() noexcept
        {
            auto current = m_stFirst.Next;
            while (current)
            {
                auto next = current->Next;
                delete current;
                current = next;
            }
        }

    private:
        /**
         * 唤醒一个对象
         */
        Holder* Awake()
        {
            auto free = m_stFirst.Next;
            if (!free)
                return new Holder();
            m_stFirst.Next = free->Next;
            free->Next = nullptr;
            return free;
        }

        /**
         * 回收一个对象
         * @param obj 对象
         */
        void Recycle(Holder* holder)
        {
            assert(holder);
            holder->Next = m_stFirst.Next;
            m_stFirst.Next = holder;
        }

    private:
        Holder m_stFirst;
    };
}
