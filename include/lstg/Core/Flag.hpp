/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
