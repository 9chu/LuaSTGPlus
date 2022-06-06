/**
 * @file
 * @author 9chu
 * @date 2022/3/5
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cassert>
#include <cstdint>

namespace lstg::Math
{
    /**
     * 随机数发生器
     * 采用 xoshiro128++ 1.0
     * @see https://prng.di.unimi.it/
     * @see https://prng.di.unimi.it/xoshiro128plusplus.c
     */
    class Randomizer
    {
    private:
        static inline uint32_t Rotate(const uint32_t x, int k) noexcept
        {
            return (x << k) | (x >> (32 - k));
        }

    public:
        /**
         * 获取种子
         * @return 种子
         */
        uint32_t GetSeed() const noexcept
        {
            return m_uSeed;
        }

        /**
         * 设置种子
         * @param seed
         */
        void SetSeed(uint32_t seed) noexcept
        {
            m_uSeed = seed;

            // 参考 Lua 的初始化方法
            // https://github.com/lua/lua/blob/8dd2c912d299b84566c6f6d659336edfa9b18e9b/lmathlib.c#L592
            m_uState[0] = seed;
            m_uState[1] = 0xFF;
            m_uState[2] = 0;
            m_uState[3] = 0;
            for (int i = 0; i < 16; i++)
                Next();
        }

        /**
         * 生成下一个随机数
         * @return [0, UINT32_MAX] 区间的随机数
         */
        inline uint32_t Next() noexcept
        {
            const uint32_t result = Rotate(m_uState[0] + m_uState[3], 7) + m_uState[0];
            const uint32_t t = m_uState[1] << 9;

            m_uState[2] ^= m_uState[0];
            m_uState[3] ^= m_uState[1];
            m_uState[1] ^= m_uState[2];
            m_uState[0] ^= m_uState[3];

            m_uState[2] ^= t;

            m_uState[3] = Rotate(m_uState[3], 11);

            return result;
        }

        /**
         * 生成 [0, n] 范围内的随机数
         * @param n 上限
         * @return 返回 [0, n] 范围内的随机整数
         */
        inline uint32_t Next(uint32_t n) noexcept
        {
            auto ran = Next();

            if ((n & (n + 1)) == 0)  // n 是 2 次方？
            {
                return ran & n;  // 位计算取有效数字
            }
            else
            {
                uint32_t lim = n;
                // 计算不小于 n 的最小 2^b - 1 数
                lim |= (lim >> 1);
                lim |= (lim >> 2);
                lim |= (lim >> 4);
                lim |= (lim >> 8);
                lim |= (lim >> 16);
                assert((lim & (lim + 1)) == 0 && lim >= n && (lim >> 1) < n);
                while ((ran &= lim) > n)
                    ran = Next();  // 不在 [0..n] 重试
                return ran;
            }
        }

        /**
         * 生成下一个随机数（浮点数）
         * @return [0, 1) 区间的浮点数
         */
        inline float NextFloat() noexcept
        {
            // 转换到一个单精度浮点可以使用表达式
            // (x >> 8) * 0x1.0p-24
            const auto x = Next();
            union {
                uint32_t i;
                float d;
            } u;
            u.i = ((0x7F) << (24 - 1)) | (x >> (32 - 24 + 1));
            return u.d - 1.0f;  // 从 [1,2) 转换到 [0,1)
        }

        /**
         * 长跳
         * 等价于 2^64 次 Next() 调用。
         */
        inline void Jump() noexcept
        {
            static const uint32_t kJump[] = { 0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b };

            uint32_t s0 = 0;
            uint32_t s1 = 0;
            uint32_t s2 = 0;
            uint32_t s3 = 0;
            for (unsigned i = 0; i < sizeof(kJump) / sizeof(*kJump); i++)
            {
                for (int b = 0; b < 32; b++)
                {
                    if (kJump[i] & UINT32_C(1) << b)
                    {
                        s0 ^= m_uState[0];
                        s1 ^= m_uState[1];
                        s2 ^= m_uState[2];
                        s3 ^= m_uState[3];
                    }
                    Next();
                }
            }

            m_uState[0] = s0;
            m_uState[1] = s1;
            m_uState[2] = s2;
            m_uState[3] = s3;
        }

        /**
         * 超长跳
         * 等价于 2^96 次 Next() 调用
         */
        inline void LongJump() noexcept
        {
            static const uint32_t kLongJump[] = { 0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662 };

            uint32_t s0 = 0;
            uint32_t s1 = 0;
            uint32_t s2 = 0;
            uint32_t s3 = 0;
            for (unsigned i = 0; i < sizeof(kLongJump) / sizeof(*kLongJump); i++)
            {
                for (int b = 0; b < 32; b++)
                {
                    if (kLongJump[i] & UINT32_C(1) << b)
                    {
                        s0 ^= m_uState[0];
                        s1 ^= m_uState[1];
                        s2 ^= m_uState[2];
                        s3 ^= m_uState[3];
                    }
                    Next();
                }
            }

            m_uState[0] = s0;
            m_uState[1] = s1;
            m_uState[2] = s2;
            m_uState[3] = s3;
        }

    private:
        uint32_t m_uSeed = 0;
        uint32_t m_uState[4];
    };
}
