/**
 * @file
 * @date 2019/12/8
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "Convert.hpp"

namespace lstg::Encoding
{
    /**
     * UTF8编解码器
     */
    class Utf8
    {
    public:
        static const char* const kName;

        class Decoder
        {
        public:
            using InputType = char;
            using OutputType = char32_t;
            static const uint32_t kMaxOutputCount = 1;

        public:
            Decoder() noexcept = default;
            Decoder(const Decoder&) noexcept = default;
            Decoder(Decoder&&) noexcept = default;

            Decoder& operator=(const Decoder&) noexcept = default;
            Decoder& operator=(Decoder&&) noexcept = default;

        public:
            bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count) noexcept
            {
                count = 0;
                auto ret = (m_iState == 0);

                m_iState = 0;
                m_iTmp = 0;
                return ret;
            }

            EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept;

        private:
            uint32_t m_iState = 0;
            uint32_t m_iTmp = 0;
        };

        class Encoder
        {
        public:
            using InputType = char32_t;
            using OutputType = char;
            static const uint32_t kMaxOutputCount = 6;

        public:
            Encoder() noexcept = default;
            Encoder(const Encoder&) noexcept = default;
            Encoder(Encoder&&) noexcept = default;

            Encoder& operator=(const Encoder&) noexcept = default;
            Encoder& operator=(Encoder&&) noexcept = default;

        public:
            bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count) noexcept
            {
                count = 0;
                return true;
            }

            EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept;
        };
    };

    /**
     * UTF16编解码器
     */
    class Utf16
    {
    public:
        static const char* const kName;

        class Decoder
        {
        public:
            using InputType = char16_t;
            using OutputType = char32_t;
            static const uint32_t kMaxOutputCount = 1;

        public:
            Decoder() noexcept = default;
            Decoder(const Decoder&) noexcept = default;
            Decoder(Decoder&&) noexcept = default;

            Decoder& operator=(const Decoder&) noexcept = default;
            Decoder& operator=(Decoder&&) noexcept = default;

        public:
            bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count) noexcept
            {
                count = 0;
                auto ret = (m_iState == 0);

                m_iState = 0;
                m_iLastWord = 0;
                return ret;
            }

            EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept;

        private:
            uint32_t m_iState = 0;
            uint16_t m_iLastWord = 0;
        };

        class Encoder
        {
        public:
            using InputType = char32_t;
            using OutputType = char16_t;
            static const uint32_t kMaxOutputCount = 2;

        public:
            Encoder() noexcept = default;
            Encoder(const Encoder&) noexcept = default;
            Encoder(Encoder&&) noexcept = default;

            Encoder& operator=(const Encoder&) noexcept = default;
            Encoder& operator=(Encoder&&) noexcept = default;

        public:
            bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count) noexcept
            {
                count = 0;
                return true;
            }

            EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept;
        };
    };

    /**
     * UTF32编解码器
     * 仅作接口匹配使用
     */
    class Utf32
    {
    public:
        static const char* const kName;

        class Decoder
        {
        public:
            using InputType = char32_t;
            using OutputType = char32_t;
            static const uint32_t kMaxOutputCount = 1;

        public:
            Decoder() noexcept = default;
            Decoder(const Decoder&) noexcept = default;
            Decoder(Decoder&&) noexcept = default;

            Decoder& operator=(const Decoder&) noexcept = default;
            Decoder& operator=(Decoder&&) noexcept = default;

        public:
            bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count) noexcept
            {
                count = 0;
                return true;
            }

            EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept
            {
                out[0] = ch;
                count = 1;
                return EncodingResult::Accept;
            }
        };

        class Encoder
        {
        public:
            using InputType = char32_t;
            using OutputType = char32_t;
            static const uint32_t kMaxOutputCount = 1;

        public:
            Encoder() noexcept = default;
            Encoder(const Encoder&) noexcept = default;
            Encoder(Encoder&&) noexcept = default;

            Encoder& operator=(const Encoder&) noexcept = default;
            Encoder& operator=(Encoder&&) noexcept = default;

        public:
            bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count) noexcept
            {
                count = 0;
                return true;
            }

            EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept
            {
                out[0] = ch;
                count = 1;
                return EncodingResult::Accept;
            }
        };
    };
}
