/**
 * @file
 * @date 2022/3/5
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "WebFileSystemError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS::detail;

const WebFileSystemErrorCategory& WebFileSystemErrorCategory::GetInstance() noexcept
{
    static const WebFileSystemErrorCategory kInstance;
    return kInstance;
}

const char* WebFileSystemErrorCategory::name() const noexcept
{
    return "WebFileSystemError";
}

std::string WebFileSystemErrorCategory::message(int ev) const
{
    switch (static_cast<WebFileSystemError>(ev))
    {
        case WebFileSystemError::Ok:
            return "ok";
        case WebFileSystemError::ApiError:
            return "general api error";
        case WebFileSystemError::HttpError:
            return "general http error";
        case WebFileSystemError::HttpHeaderFieldTooLong:
            return "http header field too long";
        case WebFileSystemError::InvalidHttpHeader:
            return "invalid http header";
        case WebFileSystemError::InvalidHttpDateTime:
            return "invalid http datetime";
        default:
            return "<unknown>";
    }
}
