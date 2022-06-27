/**
 * @file
 * @date 2017/5/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <array>
#include "../Span.hpp"
#include "../Result.hpp"
#include "EncodingError.hpp"

/**
 * 字符编码支持
 * Encoding名字空间提供对编解码器的支持。
 *
 * 约定：
 *   - 所有编解码器需要使用 编码名::Decoder 和 编码名::Encoder 实现相应的编解码器
 *   - 编解码接口必须使用 operator() 实现, 状态由类自行管理
 *   - 编解码器应当可以被拷贝或移动
 *   - 当解码发生错误时, 解码器应当返回Reject, 并重置到起始状态
 *   - 当编码发生错误时, 编码器应当返回Reject, 并重置到起始状态
 *   - 编码器不应当存储状态
 *   - 编解码器不应当抛出异常
 */
namespace lstg::Encoding
{
    /**
     * 编码结果
     */
    enum class EncodingResult
    {
        Accept = 0,
        Reject = 1,
        Incomplete = 2,
    };

    /**
     * 用于指示编码输入流终止
     */
    struct EndOfInputTag
    {
    };

    /**
     * 错误回退处理函数回调类型
     */
    template <typename EncoderOrDecoder>
    using FailureFallbackCallbackType =
        bool(std::array<typename EncoderOrDecoder::OutputType, EncoderOrDecoder::kMaxOutputCount>& out, uint32_t& count) noexcept;

    /**
     * 默认Unicode回退处理函数
     */
    inline bool DefaultUnicodeFallbackHandler(std::array<char32_t, 1>& out, uint32_t& count) noexcept
    {
        out[0] = 0xFFFD;
        count = 1;
        return true;
    }

    /**
     * 单向编码视图
     * 将编码过程迭代器化。
     * @tparam Encoder 输入编码器
     * @tparam InputType 输入字符类型
     */
    template <typename TEncoder, typename InputType = typename TEncoder::InputType>
    class EncodingView
    {
    public:
        using EncoderInputType = typename TEncoder::InputType;
        using EncoderOutputType = typename TEncoder::OutputType;
        using EncodingFailureFallbackFuncType = FailureFallbackCallbackType<TEncoder>;
        using IteratorResultType = Result<Span<const EncoderOutputType>>;

        static_assert(sizeof(EncoderInputType) == sizeof(InputType), "Character size must be equal");

        enum IteratingStates
        {
            STATE_DECODING,
            STATE_PENDING_FINISH_READING,
            STATE_FINISHED,
            STATE_ERROR,
        };

        struct Iterator : std::iterator<std::input_iterator_tag, IteratorResultType>
        {
        public:
            Iterator(Span<const InputType> src, EncodingFailureFallbackFuncType* fallback, bool end) noexcept
                : m_stSource(src), m_pFallback(fallback)
            {
                if (end)
                {
                    m_iState = STATE_FINISHED;
                    m_uPosition = m_stSource.size();
                }
                else
                {
                    this->operator++();
                }
            }

            Iterator(const Iterator&) noexcept = default;

            IteratorResultType operator*() const noexcept
            {
                if (m_iState == STATE_ERROR)
                    return make_error_code(EncodingError::EncodingFailure);
                return Span<const EncoderOutputType> {m_stBuffer.data(), m_uOutputCount};
            }

            Iterator& operator++() noexcept
            {
                if (m_iState == STATE_FINISHED)
                {
                    assert(false);
                    return *this;
                }
                else if (m_iState == STATE_ERROR)
                {
                    return *this;
                }
                else if (m_iState == STATE_PENDING_FINISH_READING)
                {
                    QX_ASSERT(m_uPosition >= m_stSource.size());
                    m_iState = STATE_FINISHED;
                    return *this;
                }

                EncodingResult result = EncodingResult::Accept;
                while (m_iState != STATE_FINISHED)
                {
                    // 读取输入
                    if (m_uPosition >= m_stSource.size())
                    {
                        // 此处需要放置EOS的标志
                        m_iState = STATE_FINISHED;
                        result = m_stEncoder(EndOfInputTag(), m_stBuffer, m_uOutputCount) ? EncodingResult::Accept : EncodingResult::Reject;
                        if (m_uOutputCount > 0)
                        {
                            assert(result == EncodingResult::Accept);
                            m_iState = STATE_PENDING_FINISH_READING;
                        }
                    }
                    else
                    {
                        result = m_stEncoder(static_cast<EncoderInputType>(m_stSource[m_uPosition++]), m_stBuffer, m_uOutputCount);
                    }

                    // 处理解码器输出
                    if (result == EncodingResult::Reject)
                    {
                        if (!m_pFallback || !m_pFallback(m_stBuffer, m_uOutputCount))
                        {
                            m_iState = STATE_ERROR;
                            return *this;
                        }
                        m_stEncoder = TEncoder();
                        result = EncodingResult::Accept;
                    }
                    if (result == EncodingResult::Accept && m_uOutputCount > 0)
                        break;
                }

                assert(m_uPosition <= m_stSource.size() && result == EncodingResult::Accept);
                return *this;
            }

            Iterator operator++(int) noexcept
            {
                Iterator tmp(*this);
                ++*this;
                return tmp;
            }

            bool operator==(const Iterator& rhs) const noexcept
            {
                return m_stSource.data() == rhs.m_stSource.data() && m_stSource.size() == rhs.m_stSource.size() &&
                    m_pFallback == rhs.m_pFallback && m_iState == rhs.m_iState && m_uPosition == rhs.m_uPosition;
            }

        private:
            Span<const InputType> m_stSource;
            EncodingFailureFallbackFuncType* m_pFallback = nullptr;

            IteratingStates m_iState = STATE_DECODING;
            size_t m_uPosition = 0;

            TEncoder m_stEncoder;
            uint32_t m_uOutputCount = 0;
            std::array<EncoderOutputType, TEncoder::kMaxOutputCount> m_stBuffer;
        };

    public:
        explicit EncodingView(Span<const InputType> src,
                              EncodingFailureFallbackFuncType* fallback = nullptr) noexcept
            : m_stSource(src), m_pFallback(fallback)
        {
        }

    public:
        Iterator begin() const noexcept
        {
            return Iterator(m_stSource, m_pFallback, false);
        }
        Iterator end() const noexcept
        {
            return Iterator(m_stSource, m_pFallback, true);
        }

    private:
        Span<const InputType> m_stSource;
        EncodingFailureFallbackFuncType* m_pFallback;
    };

    /**
     * 编码转换视图
     * 将编解码过程迭代器化。
     * @tparam InputEncoding 输入编码
     * @tparam OutputEncoding 输出编码
     * @tparam InputType 输入字符类型
     */
    template <typename InputEncoding, typename OutputEncoding, typename InputType = typename InputEncoding::Decoder::InputType>
    class ConvertingView
    {
    public:
        using DecoderType = typename InputEncoding::Decoder;
        using EncoderType = typename OutputEncoding::Encoder;
        using DecoderInputType = typename InputEncoding::Decoder::InputType;
        using DecoderOutputType = typename InputEncoding::Decoder::OutputType;
        using EncoderInputType = typename OutputEncoding::Encoder::InputType;
        using EncoderOutputType = typename OutputEncoding::Encoder::OutputType;
        using DecodingFailureFallbackFuncType = FailureFallbackCallbackType<typename InputEncoding::Decoder>;
        using EncodingFailureFallbackFuncType = FailureFallbackCallbackType<typename OutputEncoding::Encoder>;
        using IteratorResultType = Result<Span<const EncoderOutputType>>;

        static_assert(std::is_same<DecoderOutputType, EncoderInputType>::value, "InputEncoder and OutputEncoder mismatched");
        static_assert(sizeof(DecoderInputType) == sizeof(InputType), "Character size must be equal");

        enum IteratingStates
        {
            STATE_DECODING = 0,
            STATE_DECODING_PENDING_FINISH_READING = 1,
            STATE_DECODING_FINISHED = 2,
            STATE_DECODING_ERROR = 3,

            STATE_ENCODING_WAIT = 0,
            STATE_ENCODING = 1,
            STATE_ENCODING_PENDING_FINISH_READING = 2,
            STATE_ENCODING_FINISHED = 3,
            STATE_ENCODING_ERROR = 4,
        };

        struct Iterator : std::iterator<std::input_iterator_tag, IteratorResultType>
        {
        public:
            Iterator(Span<const InputType> src, DecodingFailureFallbackFuncType* decFallback, EncodingFailureFallbackFuncType* encFallback,
                bool end) noexcept
                : m_stSource(src), m_pDecodingFallback(decFallback), m_pEncodingFallback(encFallback)
            {
                if (end)
                {
                    m_iDecodingState = STATE_DECODING_FINISHED;
                    m_iEncodingState = STATE_ENCODING_FINISHED;
                    m_uDecodingPosition = m_stSource.size();
                }
                else
                {
                    this->operator++();
                }
            }

            Iterator(const Iterator&) noexcept = default;

            IteratorResultType operator*() const noexcept
            {
                if (m_iDecodingState == STATE_DECODING_ERROR)
                    return make_error_code(EncodingError::DecodingFailure);
                else if (m_iEncodingState == STATE_ENCODING_ERROR)
                    return make_error_code(EncodingError::EncodingFailure);
                return Span<const EncoderOutputType> {m_stEncodingBuffer.data(), m_uEncodingOutputCount};
            }

            Iterator& operator++() noexcept
            {
                if (m_iEncodingState == STATE_ENCODING_FINISHED)
                {
                    assert(false);
                    return *this;
                }
                else if (m_iEncodingState == STATE_ENCODING_ERROR || m_iDecodingState == STATE_DECODING_ERROR)
                {
                    return *this;
                }

                while (m_iEncodingState != STATE_ENCODING_FINISHED)
                {
                    EncodingResult result;
                    switch (m_iEncodingState)
                    {
                        case STATE_ENCODING_WAIT:
                            if (m_iDecodingState == STATE_DECODING_PENDING_FINISH_READING)
                            {
                                assert(m_uDecodingPosition >= m_stSource.size());
                                m_iDecodingState = STATE_DECODING_FINISHED;
                                m_uEncodingPosition = 0;
                                m_uEncodingOutputCount = 0;
                                m_iEncodingState = STATE_ENCODING;
                                continue;
                            }
                            assert(m_iDecodingState != STATE_DECODING_FINISHED);
                            while (m_iDecodingState != STATE_DECODING_FINISHED)
                            {
                                // 读取输入
                                if (m_uDecodingPosition >= m_stSource.size())
                                {
                                    // 此处需要放置EOS的标志
                                    m_iDecodingState = STATE_DECODING_FINISHED;
                                    result = m_stDecoder(EndOfInputTag(), m_stDecodingBuffer, m_uDecodingOutputCount) ?
                                        EncodingResult::Accept : EncodingResult::Reject;
                                    if (m_uDecodingOutputCount > 0)
                                    {
                                        assert(result == EncodingResult::Accept);
                                        m_iDecodingState = STATE_DECODING_PENDING_FINISH_READING;
                                    }
                                }
                                else
                                {
                                    result = m_stDecoder(static_cast<DecoderInputType>(m_stSource[m_uDecodingPosition++]),
                                        m_stDecodingBuffer, m_uDecodingOutputCount);
                                }

                                // 处理解码器输出
                                if (result == EncodingResult::Reject)
                                {
                                    if (!m_pDecodingFallback || !m_pDecodingFallback(m_stDecodingBuffer, m_uDecodingOutputCount))
                                    {
                                        m_iDecodingState = STATE_DECODING_ERROR;
                                        return *this;
                                    }
                                    m_stDecoder = DecoderType();
                                    m_stEncoder = EncoderType();
                                    result = EncodingResult::Accept;
                                }
                                if (result == EncodingResult::Accept && m_uDecodingOutputCount > 0)
                                    break;
                            }
                            assert(m_uDecodingPosition <= m_stSource.size() && result == EncodingResult::Accept);
                            m_uEncodingPosition = 0;
                            m_uEncodingOutputCount = 0;
                            m_iEncodingState = STATE_ENCODING;
                            [[fallthrough]];
                        default:
                            if (m_iEncodingState == STATE_ENCODING_PENDING_FINISH_READING)
                            {
                                assert(m_iDecodingState == STATE_DECODING_FINISHED);
                                assert(m_uEncodingPosition >= m_uDecodingOutputCount);
                                m_iEncodingState = STATE_ENCODING_FINISHED;
                                m_uEncodingPosition = 0;
                                m_uEncodingOutputCount = 0;
                                break;
                            }
                            assert(m_iEncodingState != STATE_ENCODING_FINISHED);
                            while (m_iEncodingState != STATE_ENCODING_FINISHED)
                            {
                                // 读取输入
                                if (m_uEncodingPosition >= m_uDecodingOutputCount)
                                {
                                    // 此处需要放置EOS的标志
                                    if (m_iDecodingState == STATE_DECODING_FINISHED)
                                    {
                                        m_iEncodingState = STATE_ENCODING_FINISHED;
                                        m_uEncodingPosition = 0;
                                        m_uEncodingOutputCount = 0;
                                        result = m_stEncoder(EndOfInputTag(), m_stEncodingBuffer, m_uEncodingOutputCount) ?
                                            EncodingResult::Accept : EncodingResult::Reject;
                                        if (m_uEncodingOutputCount > 0)
                                        {
                                            assert(result == EncodingResult::Accept);
                                            m_iEncodingState = STATE_ENCODING_PENDING_FINISH_READING;
                                        }
                                    }
                                    else
                                    {
                                        m_iEncodingState = STATE_ENCODING_WAIT;
                                        result = EncodingResult::Incomplete;
                                        break;
                                    }
                                }
                                else
                                {
                                    result = m_stEncoder(static_cast<EncoderInputType>(m_stDecodingBuffer[m_uEncodingPosition++]),
                                        m_stEncodingBuffer, m_uEncodingOutputCount);
                                }

                                // 处理解码器输出
                                if (result == EncodingResult::Reject)
                                {
                                    if (!m_pEncodingFallback || !m_pEncodingFallback(m_stEncodingBuffer, m_uEncodingOutputCount))
                                    {
                                        m_iEncodingState = STATE_ENCODING_ERROR;
                                        return *this;
                                    }
                                    m_stDecoder = DecoderType();
                                    m_stEncoder = EncoderType();
                                    result = EncodingResult::Accept;
                                }
                                if (result == EncodingResult::Accept && m_uEncodingOutputCount > 0)
                                    break;
                            }

                            // 此时，本次解码的输出全部被消耗完，但是没有编码输出，需要继续解码
                            if (result == EncodingResult::Incomplete)
                            {
                                assert(m_uDecodingPosition <= m_stSource.size());
                                assert(m_iEncodingState == STATE_ENCODING_WAIT && m_iDecodingState != STATE_DECODING_FINISHED);
                                continue;
                            }
                            assert(m_uEncodingPosition <= m_uDecodingOutputCount && result == EncodingResult::Accept);
                            return *this;
                    }
                }
                return *this;
            }

            Iterator operator++(int) noexcept
            {
                Iterator tmp(*this);
                ++*this;
                return tmp;
            }

            bool operator==(const Iterator& rhs) const noexcept
            {
                return m_stSource.data() == rhs.m_stSource.data() && m_stSource.size() == rhs.m_stSource.size() &&
                    m_pDecodingFallback == rhs.m_pDecodingFallback && m_pEncodingFallback == rhs.m_pEncodingFallback &&
                    m_iDecodingState == rhs.m_iDecodingState && m_iEncodingState == rhs.m_iEncodingState &&
                    m_uDecodingPosition == rhs.m_uDecodingPosition && m_uEncodingPosition == rhs.m_uEncodingPosition;
            }

        private:
            Span<const InputType> m_stSource;
            DecodingFailureFallbackFuncType* m_pDecodingFallback = nullptr;
            EncodingFailureFallbackFuncType* m_pEncodingFallback = nullptr;

            uint16_t m_iDecodingState = STATE_DECODING;
            uint16_t m_iEncodingState = STATE_ENCODING_WAIT;
            size_t m_uDecodingPosition = 0;
            size_t m_uEncodingPosition = 0;

            DecoderType m_stDecoder;
            uint32_t m_uDecodingOutputCount = 0;
            std::array<DecoderOutputType, InputEncoding::Decoder::kMaxOutputCount> m_stDecodingBuffer;

            EncoderType m_stEncoder;
            uint32_t m_uEncodingOutputCount = 0;
            std::array<EncoderOutputType, OutputEncoding::Encoder::kMaxOutputCount> m_stEncodingBuffer;
        };

    public:
        explicit ConvertingView(Span<const InputType> src, DecodingFailureFallbackFuncType* decFallback = nullptr,
                                EncodingFailureFallbackFuncType* encFallback = nullptr) noexcept
            : m_stSource(src), m_pDecodingFallback(decFallback), m_pEncodingFallback(encFallback)
        {
        }

    public:
        Iterator begin() const noexcept
        {
            return Iterator(m_stSource, m_pDecodingFallback, m_pEncodingFallback, false);
        }
        Iterator end() const noexcept
        {
            return Iterator(m_stSource, m_pDecodingFallback, m_pEncodingFallback, true);
        }

    private:
        Span<const InputType> m_stSource;
        DecodingFailureFallbackFuncType* m_pDecodingFallback;
        EncodingFailureFallbackFuncType* m_pEncodingFallback;
    };

    /**
     * 编码转换
     * @tparam InputEncoding 输入编码
     * @tparam OutputEncoding 输出编码
     * @tparam InputType 输入字符类型
     * @tparam OutputType 输出字符类型
     * @tparam ContainerType 容器类型
     * @param builder 转码输出
     * @param src 转码输入
     * @param decoderFailureFallback 当解码失败时的处理
     * @param encoderFailureFallback 当编码失败时的处理
     * @return 转换结果
     */
    template <typename InputEncoding, typename OutputEncoding,
              typename InputType = typename InputEncoding::Decoder::InputType,
              typename OutputType = typename OutputEncoding::Encoder::OutputType>
    Result<void> Convert(std::basic_string<OutputType>& builder, std::basic_string_view<InputType> src,
        FailureFallbackCallbackType<typename InputEncoding::Decoder> decoderFailureFallback = nullptr,
        FailureFallbackCallbackType<typename OutputEncoding::Encoder> encoderFailureFallback = nullptr) noexcept
    {
        ConvertingView<InputEncoding, OutputEncoding, InputType> view(src, decoderFailureFallback, encoderFailureFallback);

        try
        {
            builder.clear();
            builder.reserve(src.size());

            for (const auto ret : view)
            {
                if (!ret)
                    return ret.GetError();

                const auto& buf = *ret;
                for (size_t i = 0; i < buf.size(); ++i)
                    builder.push_back(static_cast<OutputType>(buf[i]));
            }
            return {};
        }
        catch (...)
        {
            return make_error_code(std::errc::not_enough_memory);
        }
    }

    /**
     * 编码转换
     * 该函数为单向转换，模板参数Encoding可以为某个编码器或者解码器。
     * @tparam Encoder 目标编码器（或者解码器）
     * @tparam InputType 输入字符类型
     * @tparam OutputType 输出字符类型
     * @tparam ContainerType 容器类型
     * @param builder 转码输出
     * @param src 转码输入
     * @param failureFallback 当解码失败时的处理
     * @return 转换结果
     */
    template <typename Encoder, typename InputType = typename Encoder::InputType,
              typename OutputType = typename Encoder::OutputType>
    Result<void> Convert(std::basic_string<OutputType>& builder, std::basic_string_view<InputType> src,
                         FailureFallbackCallbackType<Encoder> failureFallback = nullptr)
    {
        EncodingView<Encoder, InputType> view(src, failureFallback);

        try
        {
            builder.clear();
            builder.reserve(src.size());

            for (const auto ret : view)
            {
                if (!ret)
                    return ret.GetError();

                const auto& buf = *ret;
                for (size_t i = 0; i < buf.size(); ++i)
                    builder.push_back(static_cast<OutputType>(buf[i]));
            }

            return {};
        }
        catch (...)
        {
            return make_error_code(std::errc::not_enough_memory);
        }
    }
}
