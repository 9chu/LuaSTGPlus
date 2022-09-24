/**
 * @file
 * @date 2017/5/28
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Encoding/Base64.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Encoding;

const char* const Base64::kName = "Base64";

static const char kBase64EncodingTable[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};
static_assert(sizeof(kBase64EncodingTable) == 64, "Please check data");

static const uint8_t kBase64DecodingTable[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 62,   0xFF, 0xFF, 0xFF, 63,   52,   53,   54,   55,   56,   57,   58,   59,   60,
    61,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    10,
    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,
    43,   44,   45,   46,   47,   48,   49,   50,   51,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};
static_assert(sizeof(kBase64DecodingTable) == 256, "Please check data");

bool Base64::Decoder::operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count) noexcept
{
    count = 0;
    bool ret = (m_iState == 0 || m_iState == 0xFFFFFFFFu); // -1表示遇到了终止字符

    m_iState = 0;
    return ret;
}

EncodingResult Base64::Decoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept
{
    count = 0;

    // 允许消耗掉空白符
    if (ch > 0 && ::isspace(ch))
        return EncodingResult::Accept;

    // 解码流已经结束了
    if (m_iState == 0xFFFFFFFF)
    {
        m_iState = 0;
        return EncodingResult::Reject;
    }

    switch (m_iState)
    {
        case 0:
            m_stBuf[0] = static_cast<uint8_t>(ch);
            m_iState = 1;
            if (kBase64DecodingTable[m_stBuf[0]] == 0xFF)
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            break;
        case 1:
            m_stBuf[1] = static_cast<uint8_t>(ch);
            m_iState = 2;
            if (kBase64DecodingTable[m_stBuf[1]] == 0xFF)
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            break;
        case 2:
            m_stBuf[2] = static_cast<uint8_t>(ch);
            m_iState = 3;
            if (ch != '=' && kBase64DecodingTable[m_stBuf[2]] == 0xFF)
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            break;
        case 3:
            if (ch != '=' && kBase64DecodingTable[static_cast<uint8_t>(ch)] == 0xFF)
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            else
            {
                m_iState = 0;
                uint32_t tmp = kBase64DecodingTable[m_stBuf[0]] << 18u;
                tmp += kBase64DecodingTable[m_stBuf[1]] << 12u;
                out[0] = static_cast<uint8_t>((tmp & 0x00FF0000u) >> 16u);
                ++count;
                if (m_stBuf[2] != '=')
                {
                    tmp += kBase64DecodingTable[m_stBuf[2]] << 6u;
                    out[1] = static_cast<uint8_t>((tmp & 0x0000FF00u) >> 8u);
                    ++count;
                    if (ch != '=')
                    {
                        tmp += kBase64DecodingTable[static_cast<uint8_t>(ch)];
                        out[2] = static_cast<uint8_t>(tmp & 0xFFu);
                        ++count;
                    }
                    else
                    {
                        m_iState = 0xFFFFFFFF; // 输入完全处理完毕
                    }
                }
                else
                {
                    if (ch != '=')
                    {
                        m_iState = 0;
                        return EncodingResult::Reject;
                    }

                    m_iState = 0xFFFFFFFF; // 输入完全处理完毕
                }
                return EncodingResult::Accept;
            }
        default:
            assert(false);
            return EncodingResult::Reject;
    }
    return EncodingResult::Incomplete;
}

bool Base64::Encoder::operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept
{
    count = 0;
    switch (m_iState)
    {
        case 0:
            break;
        case 1:
            out[0] = kBase64EncodingTable[(m_stBuf[0] & 0xFC) >> 2];
            out[1] = kBase64EncodingTable[(m_stBuf[0] & 0x03) << 4];
            out[2] = '=';
            out[3] = '=';
            count = 4;
            break;
        case 2:
            out[0] = kBase64EncodingTable[(m_stBuf[0] & 0xFC) >> 2];
            out[1] = kBase64EncodingTable[((m_stBuf[0] & 0x03) << 4) | ((m_stBuf[1] & 0xF0) >> 4)];
            out[2] = kBase64EncodingTable[(m_stBuf[1] & 0x0F) << 2];
            out[3] = '=';
            count = 4;
            break;
        default:
            assert(false);
            return false;
    }
    return true;
}

EncodingResult Base64::Encoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
                                           uint32_t& count) noexcept
{
    count = 0;
    switch (m_iState)
    {
        case 0:
            m_stBuf[0] = ch;
            m_iState = 1;
            break;
        case 1:
            m_stBuf[1] = ch;
            m_iState = 2;
            break;
        case 2:
            out[0] = kBase64EncodingTable[(m_stBuf[0] & 0xFC) >> 2];
            out[1] = kBase64EncodingTable[((m_stBuf[0] & 0x03) << 4) + ((m_stBuf[1] & 0xF0) >> 4)];
            out[2] = kBase64EncodingTable[((m_stBuf[1] & 0x0F) << 2) + ((ch & 0xC0) >> 6)];
            out[3] = kBase64EncodingTable[ch & 0x3F];
            count = 4;
            m_iState = 0;
            return EncodingResult::Accept;
        default:
            assert(false);
            return EncodingResult::Reject;
    }
    return EncodingResult::Incomplete;
}
