/**
 * @file
 * @date 2022/5/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
        InvalidState = 7,
        DependentAssetNotFound = 8,
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

namespace std
{
    template <>
    struct is_error_code_enum<lstg::Subsystem::Asset::AssetError> : true_type {};
}
