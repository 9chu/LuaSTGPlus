/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cstdint>

#define LSTG_FLAG_BEGIN(NAME)  \
    enum class NAME : uint32_t \
    {

#define LSTG_FLAG_END(NAME) \
    };                                                                                            \
    inline NAME operator|(NAME a, NAME b) noexcept                                                \
    {                                                                                             \
        return static_cast<NAME>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));            \
    }                                                                                             \
    inline bool operator&(NAME a, NAME b) noexcept                                                \
    {                                                                                             \
        return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) == static_cast<uint32_t>(b); \
    }                                                                                             \
    inline NAME operator^(NAME a, NAME b) noexcept                                                \
    {                                                                                             \
        return static_cast<NAME>(static_cast<uint32_t>(a) & (~static_cast<uint32_t>(b)));         \
    }                                                                                             \
    inline NAME& operator|=(NAME& a, NAME b) noexcept                                             \
    {                                                                                             \
        return a = a | b;                                                                         \
    }                                                                                             \
    inline NAME& operator^=(NAME& a, NAME b) noexcept                                             \
    {                                                                                             \
        return a = a ^ b;                                                                         \
    }
