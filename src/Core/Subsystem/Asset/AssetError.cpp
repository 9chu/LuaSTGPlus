/**
 * @file
 * @date 2022/5/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Asset;

// <editor-fold desc="AssetErrorCategory">

const AssetErrorCategory& AssetErrorCategory::GetInstance() noexcept
{
    static const AssetErrorCategory kInstance;
    return kInstance;
}

const char* AssetErrorCategory::name() const noexcept
{
    return "AssetError";
}

std::string AssetErrorCategory::message(int ev) const
{
    switch (static_cast<AssetError>(ev))
    {
        case AssetError::Ok:
            return "ok";
        case AssetError::AssetAlreadyExists:
            return "asset already exists";
        case AssetError::AssetNotFound:
            return "asset not found";
        case AssetError::MissingRequiredArgument:
            return "required argument is missing";
        case AssetError::LoadingCancelled:
            return "loading task is cancelled";
        case AssetError::AssetFactoryAlreadyRegistered:
            return "asset factory is already registered";
        case AssetError::AssetFactoryNotRegistered:
            return "asset factory is not registered for this type";
        default:
            return "<unknown>";
    }
}

// </editor-fold>
