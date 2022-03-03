/**
 * @file
 * @date 2022/3/2
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include <limits>
#include <fmt/format.h>

#include "../../Result.hpp"
#include "../../Span.hpp"
#include "LuaErrorCode.hpp"
#include "Unpack.hpp"

#if defined(__clang__)
    #if __has_feature(cxx_rtti)
    #define LSTG_RTTI_ENABLED
    #endif
#elif defined(__GNUC__)
    #if defined(__GXX_RTTI)
    #define LSTG_RTTI_ENABLED
    #endif
#elif defined(_MSC_VER)
    #if defined(_CPPRTTI)
    #define LSTG_RTTI_ENABLED
    #endif
#endif

namespace lstg::Subsystem::Script
{
    template <typename T>
    class LuaClassRegister;

    namespace detail
    {
        template <typename T>
        using Unqualified = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

        template <typename T>
        struct UniqueTypeName
        {
            size_t Id;
            std::string Name;

            UniqueTypeName()
            {
                static const int kUniqueId = 0;
                Id = reinterpret_cast<size_t>(&kUniqueId);

#ifdef LSTG_RTTI_ENABLED
                const auto& info = typeid(T);
                Name = fmt::format("__{0}_{1}", info.name(), info.hash_code());
#else
                Name = fmt::format("__{0}", Id);
#endif
            }
        };

        template <typename T>
        inline const UniqueTypeName<T>& GetUniqueTypeName()
        {
            static const UniqueTypeName<T> kTypeNameInstance;
            return kTypeNameInstance;
        }
    }

    /**
     * Lua 栈封装
     */
    class LuaStack
    {
    public:
        struct AbsIndex
        {
            unsigned Index = 0;

            inline explicit AbsIndex(unsigned index) noexcept
                : Index(index)
            {}

            operator int() const noexcept
            {
                return static_cast<int>(Index);
            }

            AbsIndex& operator=(unsigned idx) noexcept
            {
                Index = idx;
                return *this;
            }

            bool operator==(unsigned idx) const noexcept
            {
                return Index == idx;
            }

            bool operator==(int idx) const noexcept
            {
                return idx >= 0 && idx == static_cast<int>(Index);
            }
        };

    public:
        LuaStack() noexcept = default;
        LuaStack(lua_State* state) noexcept
            : m_pState(state)
        {}

        inline operator const lua_State*() const noexcept
        {
            return m_pState;
        }

        inline operator lua_State*() noexcept
        {
            return m_pState;
        }

    public:
        /**
         * 获取栈顶位置
         */
        [[nodiscard]] inline unsigned GetTop() noexcept
        {
            return static_cast<unsigned>(lua_gettop(m_pState));
        }

        /**
         * 设置栈顶
         * @param sz 大小
         *
         * [-?, +?]
         */
        inline void SetTop(unsigned sz) noexcept
        {
            lua_settop(m_pState, sz);
        }

        /**
         * 获取对象类型
         * @param idx 栈索引
         * @return 类型
         *
         * [-0, +0]
         */
        [[nodiscard]] inline int TypeOf(int idx) noexcept
        {
            return lua_type(m_pState, idx);
        }

        /**
         * 从栈上弹出N个对象
         *
         * [-n, +0]
         */
        inline void Pop(unsigned n) noexcept
        {
            lua_pop(m_pState, static_cast<int>(n));
        }

        /**
         * 删除指定索引上的元素
         * @param idx 索引
         *
         * [-1, 0]
         */
        inline void Remove(int idx) noexcept
        {
            lua_remove(m_pState, idx);
        }

        /**
         * 将栈顶元素插入到指定位置
         * @param idx 绝对索引
         *
         * [-1, +1]
         */
        inline void Insert(unsigned idx) noexcept
        {
            lua_insert(m_pState, static_cast<int>(idx));
        }

        /**
         * 将元素推入栈
         * @tparam T 元素类型
         * @param arg 元素
         * @return 推入栈的元素个数
         */
        template <typename T>
        inline int PushValue(T&& arg)
        {
            return Push(*this, std::forward<T>(arg));
        }

        /**
         * 将多个值推入栈
         * @tparam T 首个被推入栈的值的类型
         * @tparam TArgs 之后被推入栈的值的类型
         * @param arg 首个元素
         * @param args 依次接在首个元素之后的元素
         * @return 被推入的元素个数
         */
        template <typename T, typename... TArgs>
        inline int PushValues(T&& arg, TArgs&&... args)
        {
            auto l = Push(std::forward<T>(arg));
            auto r = PushValues(std::forward<TArgs>(args)...);
            return l + r;
        }

        template <typename T>
        inline int PushValues(T&& arg)
        {
            return LuaPush(std::forward<T>(arg));
        }

        /**
         * 读取栈上的一个值
         * @param idx 索引（起始）
         * @return 读取的值
         *
         * [-0, +0]
         */
        template <typename T>
        inline T ReadValue(int idx)
        {
            T out;
            LuaRead(*this, idx, out);
            return out;
        }

        /**
         * 读取栈上的一个值
         * @param idx 索引（起始）
         * @param out 输出
         * @return 读取的元素个数
         *
         * [-0, +0]
         */
        template <typename T>
        inline int ReadValue(int idx, T& out)
        {
            return LuaRead(*this, idx, out);
        }

        /**
         * 对栈顶的 N 个元素执行拼接操作
         * @param n 元素
         *
         * [-n, +1]
         */
        inline void Concat(unsigned n)
        {
            lua_concat(m_pState, static_cast<int>(n));
        }

        /**
         * 在栈顶创建一个空表
         *
         * [-0, +1]
         */
        inline void NewTable()
        {
            lua_newtable(m_pState);
        }

        /**
         * 跳过 metatable 进行取值
         * @param idx 被取值的对象的索引，栈顶放置 key
         *
         * [-1, +1]
         */
        inline void RawGet(int idx) noexcept
        {
            lua_rawget(m_pState, idx);
        }

        /**
         * 跳过 metatable 进行取值
         * @param idx 被取值的对象的索引，栈顶放置 key
         * @param slot 表的下标
         *
         * [0, +1]
         */
        inline void RawGet(int idx, int slot) noexcept
        {
            lua_rawgeti(m_pState, idx, slot);
        }

        /**
         * 跳过 metatable 进行取值
         * @tparam T Key 类型
         * @param idx 被取值的对象的索引
         * @param key 键值，如果推入多个元素，只保留第一个
         *
         * [0, +1]
         */
        template <typename T>
        inline void RawGet(int idx, T&& key)
        {
            auto cnt = PushValue(std::forward<T>(key));
            assert(cnt > 0);
            if (cnt > 1)
                Pop(cnt - 1);
            RawGet(idx);
        }

        /**
         * 跳过 metatable 进行赋值
         * @param idx 被赋值对象的索引，栈顶依次放置 key, value
         *
         * [-2, +0]
         */
        inline void RawSet(int idx)
        {
            lua_rawset(m_pState, idx);
        }

        /**
         * 跳过 metatable 进行赋值
         * @param idx 被赋值对象的索引，栈顶放置 value
         * @param slot 表的下标
         *
         * [-, +0]
         */
        inline void RawSet(int idx, int slot)
        {
            lua_rawseti(m_pState, idx, slot);
        }

        /**
         * 跳过 metatable 进行赋值
         * @tparam TKey Key 类型
         * @tparam TValue 值类型
         * @param idx 被赋值对象的索引
         * @param key 键，如果推入多个元素，只保留第一个
         * @param value 值，如果推入多个元素，只保留第一个
         */
        template <typename TKey, typename TValue>
        inline void RawSet(int idx, TKey&& key, TValue&& value)
        {
            auto cnt = PushValue(std::forward<TKey>(key));
            assert(cnt > 0);
            if (cnt > 1)
                Pop(cnt - 1);

            cnt = PushValue(std::forward<TValue>(value));
            assert(cnt > 0);
            if (cnt > 1)
                Pop(cnt - 1);

            lua_rawset(m_pState, idx);
        }

        /**
         * 跳过 metatable 进行赋值
         * @tparam T 值类型
         * @param idx 被赋值对象的索引
         * @param slot 表的下标
         * @param value 值，如果推入多个元素，只保留第一个
         */
        template <typename T>
        inline void RawSet(int idx, int slot, T&& value)
        {
            auto cnt = PushValue(std::forward<T>(value));
            assert(cnt > 0);
            if (cnt > 1)
                Pop(cnt - 1);

            lua_rawseti(m_pState, idx, slot);
        }

        /**
         * 执行给定栈位置上对象的访问器方法
         * @param idx table 索引
         * @param field key
         *
         * [-0, +1]
         */
        inline void GetField(int idx, const char* field)
        {
            lua_getfield(m_pState, idx, field);
        }

        /**
         * 设置指定栈位置的表元素
         * @param idx table 索引
         * @param field key
         *
         * [-1, +0]
         */
        inline void SetField(int idx, const char* field)
        {
            lua_setfield(m_pState, idx, field);
        }

        /**
         * 设置指定栈位置的表元素
         * @tparam T 元素类型
         * @param idx table 索引
         * @param field key
         * @param value 元素
         *
         * [+0, +0]
         */
        template <typename T>
        inline void SetField(int idx, const char* field, T&& value)
        {
            auto cnt = PushValue(std::forward<T>(value));
            assert(cnt > 0);
            if (cnt > 1)
                Pop(cnt - 1);

            lua_setfield(m_pState, idx, field);
        }

        /**
         * @brief 获取一个全局值
         * @param field key
         *
         * [-0, +1]
         */
        inline void GetGlobal(const char* field)
        {
            lua_getglobal(m_pState, field);
        }

        /**
         * 设置一个全局值
         * @param field key
         *
         * [-1, +0]
         */
        inline void SetGlobal(const char* field)
        {
            lua_setglobal(m_pState, field);
        }

        /**
         * 设置一个全局值
         * @tparam T 值类型
         * @param field key
         * @param value 值
         *
         * [-0, +0]
         */
        template <typename T>
        inline void SetGlobal(const char* field, T&& value)
        {
            auto cnt = PushValue(std::forward<T>(value));
            assert(cnt > 0);
            if (cnt > 1)
                Pop(cnt - 1);

            lua_setglobal(m_pState, field);
        }

        /**
         * 设置函数的执行上下文
         * @param idx 函数或者 Thread 的索引
         *
         * [-1, +0]
         */
        inline void SetFunctionEnvironment(int idx)
        {
            lua_setfenv(m_pState, idx);
        }

        /**
         * 设置函数的执行上下文
         * @tparam T 值类型
         * @param idx 函数或者 Thread 的索引
         * @param value 上下文
         */
        template <typename T>
        inline void SetFunctionEnvironment(int idx, T&& value)
        {
            auto cnt = PushValue(std::forward<T>(value));
            assert(cnt > 0);
            if (cnt > 1)
                Pop(cnt - 1);

            lua_setfenv(m_pState, idx);
        }

        /**
         * 抛出一个错误
         *
         * 注意，Lua 默认采取 C 的 longjmp 进行错误抛出。
         * 在 Linux 环境下需要注意 C++ unwind 可能不会正常调用从而造成内存泄漏。
         *
         * @tparam TArgs 格式化参数类型
         * @param fmt 格式化文本
         * @param args 参数
         */
        template <typename... TArgs>
        [[noreturn]] inline void Error(const char* fmt, TArgs&&... args)
        {
            luaL_error(m_pState, fmt, std::forward<TArgs>(args)...);
        }

        /**
         * 调用函数
         * @param nargs 参数个数
         * @param nrets 返回值个数
         *
         * [-(nargs+1), +nrets]
         */
        inline void Call(unsigned nargs, unsigned nrets)
        {
            lua_call(m_pState, static_cast<int>(nargs), static_cast<int>(nrets));
        }

        /**
         * 调用栈顶函数
         * @tparam T 返回值类型
         * @tparam TArgs 参数类型
         * @param args 参数
         * @return 返回值
         *
         * [-1, +0]
         */
        template <typename T, typename... TArgs>
        inline T Call(TArgs&&... args)
        {
            if constexpr (std::is_same_v<T, void>)
            {
                auto nargs = PushValues(std::forward<TArgs>(args)...);
                lua_call(m_pState, nargs, 0);
            }
            else
            {
                auto nargs = PushValues(std::forward<TArgs>(args)...);
                lua_call(m_pState, nargs, detail::CountArgs<T>::value);

                {
                    T ret;
                    auto nret = ReadValue(-detail::CountArgs<T>::value, ret);
                    static_cast<void>(nret);
                    assert(nret == detail::CountArgs<T>::value);
                    lua_pop(m_pState, nret);
                    return ret;
                }
            }
        }

        /**
         * 安全调用函数
         * @param nargs 参数个数
         * @param nrets 返回值个数
         * @param errFunc 错误处理函数的绝对索引
         * @return 返回值
         *
         * [-(nargs + 1), +(nresults|1)]
         */
        Result<void> ProtectedCall(unsigned nargs, unsigned nrets, unsigned errFunc = 0) noexcept
        {
            auto ret = lua_pcall(m_pState, static_cast<int>(nargs), static_cast<int>(nrets), static_cast<int>(errFunc));
            if (ret == 0)
                return {};
            return make_error_code(static_cast<LuaError>(ret));
        }

        /**
         * 安全调用栈顶函数
         * @tparam T 返回值类型
         * @tparam TArgs 参数类型
         * @param args 参数
         * @return 返回值
         *
         * [-1, 0 | +1]
         */
        template <typename T, typename... TArgs>
        inline Result<T> ProtectedCall(TArgs&&... args, unsigned errFunc = 0)
        {
            if constexpr (std::is_same_v<T, void>)
            {
                auto nargs = PushValues(std::forward<TArgs>(args)...);
                auto ret = lua_pcall(m_pState, nargs, 0, errFunc);
                if (ret == 0)
                    return {};
                return make_error_code(static_cast<LuaError>(ret));
            }
            else
            {
                auto nargs = PushValues(std::forward<TArgs>(args)...);
                auto ret = lua_pcall(m_pState, nargs, detail::CountArgs<T>::value, errFunc);
                if (ret != 0)
                {
                    return make_error_code(static_cast<LuaError>(ret));
                }
                else
                {
                    T retValue;
                    auto nret = ReadValue(-detail::CountArgs<T>::value, retValue);
                    static_cast<void>(nret);
                    assert(nret == detail::CountArgs<T>::value);
                    return { OkTag {}, std::move(retValue) };
                }
            }
        }

        /**
         * 加载缓冲区
         * @param content 内容
         * @param name 名称
         * @return 是否成功
         *
         * 若失败，栈上存储错误说明字符串。
         *
         * [-0, +1]
         */
        inline Result<void> LoadBuffer(Span<const uint8_t> content, const char* name = "")
        {
            auto ret = luaL_loadbuffer(m_pState, reinterpret_cast<const char*>(content.data()), content.size(), name);
            if (ret == 0)
                return {};
            return make_error_code(static_cast<LuaError>(ret));
        }

        /**
         * 从字符串编译
         * @param content 内容
         * @return 是否成功
         *
         * 若失败，栈上存储错误说明字符串。
         *
         * [-0, +1]
         */
        inline Result<void> LoadString(const char* content)
        {
            auto ret = luaL_loadstring(m_pState, content);
            if (ret == 0)
                return {};
            return make_error_code(static_cast<LuaError>(ret));
        }

    protected:
        ::lua_State* m_pState = nullptr;
    };
}