/**
 * @file
 * @date 2022/3/2
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <tuple>

namespace lstg::Subsystem::Script
{
    /**
     * Unpack 类型标记
     * 用于指示 Lua 返回值等在推入堆栈时先解包
     * @tparam TArgs 参数类型
     */
    template <typename... TArgs>
    struct Unpack :
        public std::tuple<TArgs...>
    {
    public:
        using std::tuple<TArgs...>::tuple;
    };

    class LuaStack;

    namespace detail
    {
        template <typename T>
        struct IsUnpack :
            public std::false_type {};

        template <typename... TArgs>
        struct IsUnpack<Unpack<TArgs...>> :
            public std::true_type {};

        template <typename T>
        struct CountArgs
        {
            static constexpr int Value = 1;
        };

        template <typename T>
        struct CountArgs<Unpack<T>>
        {
            static constexpr int Value = CountArgs<T>::Value;
        };

        template <typename T, typename... TArgs>
        struct CountArgs<Unpack<T, TArgs...>>
        {
            static constexpr int Value = CountArgs<T>::Value + CountArgs<Unpack<TArgs...>>::Value;
        };

        template <>
        struct CountArgs<LuaStack>
        {
            static constexpr int Value = 0;
        };

        template <>
        struct CountArgs<const LuaStack>
        {
            static constexpr int Value = 0;
        };

        template <>
        struct CountArgs<LuaStack&>
        {
            static constexpr int Value = 0;
        };

        template <>
        struct CountArgs<const LuaStack&>
        {
            static constexpr int Value = 0;
        };

        template <int... Indices>
        struct StackIndexSequence {};

        template <typename T1, typename T2>
        struct StackIndexMerger;

        template <int... Indices1, int... Indices2>
        struct StackIndexMerger<StackIndexSequence<Indices1...>, StackIndexSequence<Indices2...>>
        {
            using Sequence = StackIndexSequence<Indices1..., Indices2...>;
        };

        template <int StartIndex = 1, typename... TArgs>
        struct MakeStackIndexSequence;

        template <int StartIndex>
        struct MakeStackIndexSequence<StartIndex>
        {
            using Sequence = StackIndexSequence<>;
        };

        template <int StartIndex, typename T>
        struct MakeStackIndexSequence<StartIndex, T>
        {
            using Sequence = StackIndexSequence<StartIndex>;
        };

        template <int StartIndex, typename T, typename... Rest>
        struct MakeStackIndexSequence<StartIndex, T, Rest...>
        {
            using Sequence = typename StackIndexMerger<
                    typename MakeStackIndexSequence<StartIndex, T>::Sequence,
                    typename MakeStackIndexSequence<StartIndex + CountArgs<T>::Value, Rest...>::Sequence
                >::Sequence;
        };
    }
}
