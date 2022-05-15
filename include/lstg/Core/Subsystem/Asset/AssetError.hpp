/**
 * @file
 * @date 2022/5/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <system_error>

namespace lstg::Subsystem::Asset
{
    /**
     * 资产系统错误代码
     */
    enum class AssetError
    {
        Ok = 0,
        AssetAlreadyExists = 1,
        AssetNotFound = 2,
        MissingRequiredArgument = 3,
        LoadingCancelled = 4,
        AssetFactoryAlreadyRegistered = 5,
        AssetFactoryNotRegistered = 6,
    };

    /**
     * 资产系统错误代码分类
     */
    class AssetErrorCategory : public std::error_category
    {
    public:
        static const AssetErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(AssetError ec) noexcept
    {
        return { static_cast<int>(ec), AssetErrorCategory::GetInstance() };
    }
}
