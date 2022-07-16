/**
 * @file
 * @date 2022/6/5
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string>
#include <string_view>
#include <lstg/Core/Result.hpp>

namespace lstg::v2
{
    /**
     * 资产类型
     */
    enum class AssetTypes
    {
        Texture = 1,
        Image = 2,
        Animation = 3,
        Music = 4,
        Sound = 5,
        Particle = 6,
        TexturedFont = 7,
        TrueTypeFont = 8,
        Effect = 9,
    };

    /**
     * 转换资产类型到前缀
     * @param t 资产类型
     * @return 前缀
     */
    const char* AssetTypeToPrefix(AssetTypes t) noexcept;

    /**
     * 检查资产名是否与类型匹配
     * @param name 名称
     * @param t 类型
     * @return 是否匹配
     */
    bool IsAssetNameMatchType(std::string_view name, AssetTypes t) noexcept;

    /**
     * 构造完整的资产名
     * @param t 资产类型
     * @param name 名称
     * @return 完整资产名
     */
    std::string MakeFullAssetName(AssetTypes t, std::string_view name);

    /**
     * 构造完整的资产名（原地）
     * @param out 输出缓冲区
     * @param t 资产类型
     * @param name 名称
     */
    Result<void> MakeFullAssetName(std::string& out, AssetTypes t, std::string_view name) noexcept;

    /**
     * 解出资产名
     * @param name 完整资产名
     * @return 脚本系统中所用资产名
     */
    std::string ExtractAssetName(std::string_view name);
}
