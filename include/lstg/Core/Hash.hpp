/**
 * @file
 * @date 2022/4/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "Span.hpp"

namespace lstg
{
    /**
     * MurmurHash 3
     * @param input 输入数据
     * @return 输出哈希值
     */
    inline uint32_t MurmurHash3(Span<const uint8_t> input, uint32_t seed = 0) noexcept
    {
#define ROTL32(x, r) (x << r) | (x >> (32u - r))

        auto data = input.data();
        const int blocksCount = input.size() / 4;

        uint32_t h1 = seed;
        const uint32_t c1 = 0xcc9e2d51;
        const uint32_t c2 = 0x1b873593;

        const uint32_t* blocks = (const uint32_t*)(data + blocksCount * 4);
        for(int i = -blocksCount; i; i++)
        {
            // 如有必要，在这里处理大小端和内存对齐
            // 这里假定了平台能直接访问四字节对齐的内存地址
            uint32_t k1 = blocks[i];

            k1 *= c1;
            k1 = ROTL32(k1, 15u);
            k1 *= c2;

            h1 ^= k1;
            h1 = ROTL32(h1, 13u);
            h1 = h1 * 5 + 0xe6546b64;
        }

        const uint8_t* tail = data + blocksCount * 4;
        uint32_t k1 = 0;
        switch (input.GetSize() & 3)
        {
            case 3:
                k1 ^= tail[2] << 16;
            case 2:
                k1 ^= tail[1] << 8;
            case 1:
                k1 ^= tail[0];
                k1 *= c1;
                k1 = ROTL32(k1, 15u);
                k1 *= c2;
                h1 ^= k1;
        };

        h1 ^= input.size();

        // fmix32(h1)
        h1 ^= h1 >> 16;
        h1 *= 0x85ebca6b;
        h1 ^= h1 >> 13;
        h1 *= 0xc2b2ae35;
        h1 ^= h1 >> 16;
        return h1;

#undef ROTL32
    }
}
