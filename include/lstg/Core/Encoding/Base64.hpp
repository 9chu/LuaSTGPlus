/**
 * @file
 * @date 2019/12/8
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "Convert.hpp"

namespace lstg::Encoding
{
    /**
     * BASE64编解码器
     */
    class Base64
    {
    public:
        static const char* const kName;

        class Decoder
        {
        public:
            using InputType = char;
            using OutputType = uint8_t;
            static const uint32_t kMaxOutputCount = 3;

        public:
            Decoder() noexcept = default;
            Decoder(const Decoder&) noexcept = default;
            Decoder(Decoder&&) noexcept = default;

            Decoder& operator=(const Decoder&) noexcept = default;
            Decoder& operator=(Decoder&&) noexcept = default;

        public:
            bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept;
            EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept;

        private:
            uint32_t m_iState = 0;
            std::array<uint8_t, 3> m_stBuf;
        };

        class Encoder
        {
        public:
            using InputType = uint8_t;
            using OutputType = char;
            static const uint32_t kMaxOutputCount = 4;

        public:
            Encoder() noexcept = default;
            Encoder(const Encoder&) noexcept = default;
            Encoder(Encoder&&) noexcept = default;

            Encoder& operator=(const Encoder&) noexcept = default;
            Encoder& operator=(Encoder&&) noexcept = default;

        public:
            bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept;
            EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept;

        private:
            uint32_t m_iState = 0;
            std::array<char, 2> m_stBuf;
        };
    };
}
