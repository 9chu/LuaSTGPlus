/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "ZLibError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS::detail;

const ZLibErrorCategory& ZLibErrorCategory::GetInstance() noexcept
{
    static const ZLibErrorCategory kCategory;
    return kCategory;
}

const char* ZLibErrorCategory::name() const noexcept
{
    return "ZLibError";
}

std::string ZLibErrorCategory::message(int ev) const
{
    switch (static_cast<ZLibError>(ev))
    {
        case ZLibError::Ok:
            return "ok";
        case ZLibError::StreamEnd:
            return "end of stream reached";
        case ZLibError::NeedDict:
            return "preset dictionary required";
        case ZLibError::GeneralError:
            return "i/o error occur";
        case ZLibError::StreamError:
            return "invalid stream or parameters";
        case ZLibError::DataError:
            return "invalid data";
        case ZLibError::MemoryError:
            return "no available memory";
        case ZLibError::BufferError:
            return "additional buffers are required";
        case ZLibError::VersionError:
            return "version mismatched";
        default:
            return "<unknown>";
    }
}
