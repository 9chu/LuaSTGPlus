/**
 * @file
 * @date 2022/2/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>

namespace lstg
{
    namespace detail
    {
        template <class T, class E, class = void>
        struct IsContiguousContainer :
            std::false_type {};

        template <class T, class E>
        struct IsContiguousContainer<T, E, std::void_t<
            decltype(
                std::declval<std::size_t&>() = std::declval<T const&>().size(),
                    std::declval<E*&>() = std::declval<T&>().data()
            ),
            typename std::enable_if<
                std::is_same<
                    typename std::remove_cv<E>::type,
                    typename std::remove_cv<
                        typename std::remove_pointer<
                            decltype(std::declval<T&>().data())
                        >::type
                    >::type
                >::value
            >::type>> :
            std::true_type {};
    }

    /**
     * 无所有权连续容器
     * @tparam T 类型
     */
    template <typename T>
    class Span
    {
    public:
        using ElementType = T;
        using ValueType = typename std::remove_const<T>::type;
        using IndexType = std::ptrdiff_t;
        using Pointer = T*;
        using Reference = T&;
        using Iterator = Pointer;
        using ConstPointer = T const*;
        using ConstReference = T const&;
        using ConstIterator = ConstPointer;

    public:
        Span() noexcept = default;
        Span(const Span&) noexcept = default;
        Span& operator=(const Span&) noexcept = default;

        Span(T* data, std::size_t size) noexcept
            : m_pData(data), m_uSize(size)
        {
        }

        template <class ContiguousContainer,
            class = typename std::enable_if<detail::IsContiguousContainer<ContiguousContainer, T>::value>::type>
        explicit Span(ContiguousContainer&& container) noexcept
            : m_pData(container.data()), m_uSize(container.size())
        {
        }

        template <class CharT, class Traits, class Allocator>
        explicit Span(std::basic_string<CharT, Traits, Allocator>& s) noexcept
            : m_pData(&s[0]), m_uSize(s.size())
        {
        }

        template <class CharT, class Traits, class Allocator>
        explicit Span(const std::basic_string<CharT, Traits, Allocator>& s) noexcept
            : m_pData(s.data()), m_uSize(s.size())
        {
        }

        template <class ContiguousContainer>
        typename std::enable_if<detail::IsContiguousContainer<ContiguousContainer, T>::value, Span&>::type
        operator=(ContiguousContainer&& container) noexcept
        {
            m_pData = container.data();
            m_uSize = container.size();
            return *this;
        }

        template <class CharT, class Traits, class Allocator>
        Span& operator=(std::basic_string<CharT, Traits, Allocator>& s) noexcept
        {
            m_pData = &s[0];
            m_uSize = s.size();
            return *this;
        }

        template <class CharT, class Traits, class Allocator>
        Span& operator=(const std::basic_string<CharT, Traits, Allocator>& s) noexcept
        {
            m_pData = s.data();
            m_uSize = s.size();
            return *this;
        }

        Reference operator[](std::size_t index) noexcept
        {
            assert(index < m_uSize);
            return m_pData[index];
        }

        ConstReference operator[](std::size_t index) const noexcept
        {
            assert(index < m_uSize);
            return m_pData[index];
        }

    public:
        [[nodiscard]] bool IsEmpty() const noexcept
        {
            return m_uSize == 0;
        }

        [[nodiscard]] T* GetData() const noexcept
        {
            return m_pData;
        }

        [[nodiscard]] std::size_t GetSize() const noexcept
        {
            return m_uSize;
        }

        [[nodiscard]] Iterator Begin() noexcept
        {
            return m_pData;
        }

        [[nodiscard]] Iterator End() noexcept
        {
            return m_pData + m_uSize;
        }

        [[nodiscard]] ConstIterator ConstBegin() const noexcept
        {
            return m_pData;
        }

        [[nodiscard]] ConstIterator ConstEnd() const noexcept
        {
            return m_pData + m_uSize;
        }

        Span<T> Slice(size_t start, size_t end) const noexcept
        {
            auto startPos = std::min<ConstIterator>(m_pData + start, m_pData + m_uSize);
            auto endPos = std::max<ConstIterator>(startPos, std::min(m_pData + end, m_pData + m_uSize));
            return Span<T>(startPos, endPos - startPos);
        }

    public:  // std 兼容接口
        [[nodiscard]] size_t size() const noexcept { return GetSize(); }
        [[nodiscard]] T* data() noexcept { return GetData(); }
        [[nodiscard]] const T* data() const noexcept { return GetData(); }

        [[nodiscard]] Iterator begin() noexcept { return Begin(); }
        [[nodiscard]] Iterator end() noexcept { return End(); }

    private:
        T* m_pData = nullptr;
        std::size_t m_uSize = 0u;
    };
}
