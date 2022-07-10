/**
 * @file
 * @date 2022/7/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "HgeFontFactory.hpp"

extern "C" {
#include <ryu/ryu_parse.h>
}
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Encoding/Unicode.hpp>
#include <lstg/Core/Text/IniSaxParser.hpp>
#include "HgeFontFace.hpp"
#include "detail/HgeFontLoadError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;
using namespace lstg::Subsystem::Render::Font::detail;

LSTG_DEF_LOG_CATEGORY(HgeFontFactory);

namespace
{
//    Result<string> ReadWholeFile(Subsystem::VFS::IStream* stream) noexcept
//    {
//        static const size_t kExpand = 16 * 1024;
//
//        assert(stream);
//        string ret;
//        try
//        {
//            while (true)
//            {
//                auto sz = ret.size();
//                ret.resize(sz + kExpand);
//
//                auto err = stream->Read(reinterpret_cast<uint8_t*>(ret.data() + sz), ret.size() - sz);
//                if (!err)
//                    return err.GetError();
//
//                ret.resize(sz + *err);
//                if (*err < kExpand)
//                    break;
//            }
//        }
//        catch (...)
//        {
//            return make_error_code(errc::not_enough_memory);
//        }
//        return ret;
//    }

    class FontGenerator :
        public Text::IIniSaxListener
    {
        enum  {
            STATE_LOOKFOR_SECTION,
            STATE_READING_SECTION,
            STATE_FINISH_SECTION,
        };

    public:
        FontGenerator(std::shared_ptr<HgeFontFace> p, IFontDependencyLoader* dependencyLoader) noexcept
            : m_pFontFace(std::move(p)), m_pDependencyLoader(dependencyLoader)
        {
            assert(m_pDependencyLoader);
        }

    public:  // IIniSaxListener
        Result<void> OnSectionBegin(std::string_view name) noexcept override
        {
            switch (m_iState)
            {
                case STATE_LOOKFOR_SECTION:
                    if (name == "HGEFONT")
                        m_iState = STATE_READING_SECTION;
                    break;
                case STATE_FINISH_SECTION:
                    if (name == "HGEFONT")
                        return make_error_code(HgeFontLoadError::DuplicatedFontSection);
                    break;
                default:
                    assert(false);
                    break;
            }
            return {};
        }

        Result<void> OnSectionEnd() noexcept override
        {
            switch (m_iState)
            {
                case STATE_READING_SECTION:
                    m_iState = STATE_FINISH_SECTION;
                    break;
                default:
                    break;
            }
            return {};
        }

        Result<void> OnValue(std::string_view key, std::string_view value) noexcept override
        {
            if (key == "Bitmap")
            {
                if (m_bBitmapRead)
                    return make_error_code(HgeFontLoadError::DuplicatedBitmap);

                auto ret = m_pDependencyLoader->OnLoadTexture(value);
                if (!ret)
                {
                    LSTG_LOG_ERROR_CAT(HgeFontFactory, "Load texture \"{}\" fail", value);
                    return ret.GetError();
                }

                m_pFontFace->SetAtlasTexture(std::move(*ret));
                m_bBitmapRead = true;
            }
            else if (key == "Char")
            {
                // 读取 char
                enum {
                    STATE_START,
                    STATE_READ_CHAR,
                    STATE_LOOKING_FINISH,
                    STATE_READ_HEX_NUMBER,
                    STATE_FINISH,
                } state = STATE_START;
                size_t i = 0;
                size_t chStart = 0;
                uint32_t chNumber = 0;
                while (i <= value.length() && state != STATE_FINISH)
                {
                    auto ch = (i >= value.length() ? '\0' : value[i]);
                    switch (state)
                    {
                        case STATE_START:
                            if (ch == '"')
                            {
                                state = STATE_READ_CHAR;
                                chStart = i + 1;
                                break;
                            }
                            else if (('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F'))
                            {
                                state = STATE_READ_HEX_NUMBER;
                                continue;
                            }
                            else
                            {
                                LSTG_LOG_ERROR_CAT(HgeFontFactory, "Unexpected character '{}'", ch);
                                return make_error_code(HgeFontLoadError::UnexpectedCharacter);
                            }
                            break;
                        case STATE_READ_CHAR:
                            if (chStart != i && ch == '"')
                            {
                                state = STATE_LOOKING_FINISH;

                                // 对字符进行 UTF-8 解码
                                auto utf8Ch = string_view { value.data() + chStart, i - chStart };
                                Encoding::Utf8::Decoder decoder;
                                for (size_t j = 0; j <= utf8Ch.size(); ++j)
                                {
                                    uint32_t decodedCnt = 0;
                                    array<char32_t, 1> decodedOut = { 0 };
                                    if (j >= utf8Ch.size())
                                    {
                                        auto err = decoder(Encoding::EndOfInputTag{}, decodedOut, decodedCnt);
                                        if (!err)
                                        {
                                            LSTG_LOG_ERROR_CAT(HgeFontFactory, "Can't decode utf-8 string \"{}\"", utf8Ch);
                                            return make_error_code(HgeFontLoadError::Utf8DecodeError);
                                        }
                                    }
                                    else
                                    {
                                        auto err = decoder(utf8Ch[j], decodedOut, decodedCnt);
                                        if (err == Encoding::EncodingResult::Accept)
                                        {
                                            assert(decodedCnt == 1);
                                            chNumber = decodedOut[0];
                                            break;
                                        }
                                        else if (err == Encoding::EncodingResult::Reject)
                                        {
                                            LSTG_LOG_ERROR_CAT(HgeFontFactory, "Can't decode utf-8 string \"{}\"", utf8Ch);
                                            return make_error_code(HgeFontLoadError::Utf8DecodeError);
                                        }
                                    }
                                }
                            }
                            else if (ch == '\0')
                            {
                                LSTG_LOG_ERROR_CAT(HgeFontFactory, "Unexpected EOF");
                                return make_error_code(HgeFontLoadError::UnexpectedCharacter);
                            }
                            break;
                        case STATE_LOOKING_FINISH:
                            if (ch == ',' || ch == '\0')
                            {
                                state = STATE_FINISH;
                                continue;
                            }
                            else
                            {
                                LSTG_LOG_ERROR_CAT(HgeFontFactory, "Unexpected character '{}'", ch);
                                return make_error_code(HgeFontLoadError::UnexpectedCharacter);
                            }
                            break;
                        case STATE_READ_HEX_NUMBER:
                            if (ch >= '0' && ch <= '9')
                            {
                                chNumber = chNumber * 16 + (ch - '0');
                            }
                            else if (ch >= 'a' && ch <= 'f')
                            {
                                chNumber = chNumber * 16 + (ch - 'a') + 10;
                            }
                            else if (ch >= 'A' && ch <= 'F')
                            {
                                chNumber = chNumber * 16 + (ch - 'A') + 10;
                            }
                            else if (ch == ',' || ch == '\0')
                            {
                                state = STATE_FINISH;
                                continue;
                            }
                            break;
                        default:
                            assert(false);
                            break;
                    }
                    ++i;
                }

                assert(i >= value.length() || value[i] == ',');
                if (i >= value.length())
                {
                    LSTG_LOG_ERROR_CAT(HgeFontFactory, "Glyph info expected, but EOF read");
                    return make_error_code(HgeFontLoadError::UnexpectedCharacter);
                }
                ++i;  // 跳过 ','

                // 读取后续的 Glyph 定义
                size_t floatStart = i;
                size_t valueCnt = 0;
                float values[6] = { 0, 0, 0, 0, 0, 0 };  // x, y, w, h, left_offset, right_offset
                while (i <= value.length())
                {
                    auto ch = (i >= value.length() ? '\0' : value[i]);
                    if (ch == ',' || ch == '\0')
                    {
                        float v = 0;
                        string_view floatText = { value.data() + floatStart, i - floatStart };
                        auto err = ::s2f_n(floatText.data(), static_cast<int>(floatText.length()), &v);
                        if (err != SUCCESS)
                        {
                            LSTG_LOG_ERROR_CAT(HgeFontFactory, "Parse value \"{}\" fail", floatText);
                            return make_error_code(HgeFontLoadError::InvalidValue);
                        }
                        values[valueCnt++] = v;
                        if (valueCnt >= std::extent_v<decltype(values)>)
                            break;
                        floatStart = i + 1;
                    }
                    ++i;
                }
                if (valueCnt < std::extent_v<decltype(values)>)
                {
                    LSTG_LOG_ERROR_CAT(HgeFontFactory, "Invalid glyph info");
                    return make_error_code(HgeFontLoadError::InvalidValue);
                }

                // 计入结果
                auto ret = m_pFontFace->AppendGlyph(static_cast<char32_t>(chNumber), values[0], values[1], values[2], values[3], values[4],
                    values[5]);
                if (!ret)
                    return ret.GetError();
            }
            return {};
        }

    private:
        std::shared_ptr<HgeFontFace> m_pFontFace;
        IFontDependencyLoader* m_pDependencyLoader = nullptr;
        int32_t m_iState = STATE_LOOKFOR_SECTION;
        bool m_bBitmapRead = false;
    };
}

Result<FontFacePtr> HgeFontFactory::CreateFontFace(VFS::StreamPtr stream, IFontDependencyLoader* dependencyLoader, int faceIndex) noexcept
{
    assert(dependencyLoader);
    if (!dependencyLoader)
        return make_error_code(errc::invalid_argument);
    if (faceIndex != 0)
        return make_error_code(errc::invalid_argument);

    string content;
    auto ret = ReadAll(content, stream.get());
    if (!ret)
        return ret.GetError();

    auto face = make_shared<HgeFontFace>();
    FontGenerator gen(face, dependencyLoader);
    ret = Text::IniSaxParser::Parse(content, &gen,
        Text::IniParsingFlags::IgnoreKeyLeadingSpaces | Text::IniParsingFlags::IgnoreKeyTailingSpaces |
        Text::IniParsingFlags::IgnoreValueLeadingSpaces | Text::IniParsingFlags::IgnoreValueTailingSpaces |
        Text::IniParsingFlags::IgnoreSectionLeadingSpaces | Text::IniParsingFlags::IgnoreSectionTailingSpaces);
    if (!ret)
        return ret.GetError();
    return face;
}

Result<size_t> HgeFontFactory::EnumFontFace(std::vector<FontFaceInfo>& out, VFS::StreamPtr stream) noexcept
{
    return make_error_code(errc::not_supported);
}
