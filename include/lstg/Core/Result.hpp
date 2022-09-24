/**
 * @file
 * @date 2021/10/18
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cassert>
#include <type_traits>
#include <variant>
#include <system_error>

namespace lstg
{
    template <typename T>
    class Result;

    namespace detail
    {
        template <typename T>
        struct IsErrorCode
        {
            static const bool value = std::is_same<std::remove_const_t<std::remove_reference_t<T>>, std::error_code>::value;
        };

        template <typename T>
        struct IsResultNested : public std::false_type
        {
        };

        template <typename P>
        struct IsResultNested<Result<P>> : public std::true_type
        {
        };

        template <typename T>
        struct IsResult
        {
            static const bool value = IsResultNested<std::remove_const_t<std::remove_reference_t<T>>>::value;
        };

        template <typename T>
        struct IsNotResultNorErrorCode
        {
            static const bool value = !IsResult<T>::value && !IsErrorCode<T>::value;
        };
    } // namespace detail

    struct OkTag {};

    /**
     * 一个返回错误码或者结果的包装类。
     * @tparam T 结果的类型，不能是 std::error_code
     */
    template <typename T>
    class Result
    {
    public:
        /**
         * 通过 error_code 构造
         * @param ec 错误码
         */
        Result(std::error_code ec) noexcept
            : m_stValue(ec)
        {
            assert(ec); // 保存结果时，总是应该满足 ec != {}
        }

        /**
         * 构造结果
         * @tparam P 参数类型
         * @param obj 参数
         */
        template <typename P>
        Result(P&& obj, typename std::enable_if<lstg::detail::IsNotResultNorErrorCode<P>::value, void*>::type = nullptr) noexcept
            : m_stValue(std::forward<P>(obj))
        {
        }

        /**
         * 构造结果
         * @tparam P 参数类型
         * @param obj 参数
         */
        template <typename P>
        Result(OkTag, P&& obj) noexcept
            : m_stValue(std::in_place_index_t<1>{}, std::forward<P>(obj))
        {
        }

        Result(const Result&) = default;
        Result(Result&&) noexcept = default;

        Result& operator=(const Result&) = default;
        Result& operator=(Result&&) noexcept = default;

    public:
        /**
         * 检查是否存在结果
         * @return true 成功, false 失败
         */
        operator bool() const noexcept
        {
            return !HasError();
        }

        /**
         * 获取结果
         */
        const T& operator*() const noexcept
        {
            assert(m_stValue.index() == 1);
            return std::get<1>(m_stValue);
        }
        T& operator*() noexcept
        {
            assert(m_stValue.index() == 1);
            return std::get<1>(m_stValue);
        }

        /**
         * 获取结果
         */
        const T* operator->() const noexcept
        {
            assert(m_stValue.index() == 1);
            return &std::get<1>(m_stValue);
        }
        T* operator->() noexcept
        {
            assert(m_stValue.index() == 1);
            return &std::get<1>(m_stValue);
        }

        /**
         * 检查是否存在错误
         */
        bool HasError() const noexcept
        {
            return m_stValue.index() == 0;
        }

        /**
         * 检查是否存在结果
         */
        bool HasResult() const noexcept
        {
            return m_stValue.index() == 1;
        }

        /**
         * 获取错误对象
         */
        std::error_code GetError() const noexcept
        {
            return std::get<0>(m_stValue);
        }

        /**
         * 如果有错误则抛出 system_error 异常
         */
        const T& ThrowIfError() const
        {
            if (HasError())
                throw std::system_error(GetError());
            return std::get<1>(m_stValue);
        }
        T& ThrowIfError()
        {
            if (HasError())
                throw std::system_error(GetError());
            return std::get<1>(m_stValue);
        }

    private:
        std::variant<std::error_code, T> m_stValue;
    };

    template <>
    class Result<void>
    {
    public:
        /**
         * 通过 error_code 构造
         * @param ec 错误码
         */
        Result(std::error_code ec = {})
            : m_stValue(ec)
        {
        }

        Result(const Result&) noexcept = default;
        Result(Result&&) noexcept = default;

        Result& operator=(const Result&) noexcept = default;
        Result& operator=(Result&&) noexcept = default;

    public:
        /**
         * 检查是否存在结果
         */
        operator bool() const noexcept
        {
            return !HasError();
        }

        /**
         * 检查是否存在错误
         */
        bool HasError() const noexcept
        {
            return !!m_stValue;
        }

        /**
         * 获取错误对象
         */
        std::error_code GetError() const noexcept
        {
            return m_stValue;
        }

        /**
         * 如果有错误则抛出 system_error 异常
         */
        void ThrowIfError() const
        {
            if (HasError())
                throw std::system_error(GetError());
        }

    private:
        std::error_code m_stValue;
    };
} // namespace lstg
