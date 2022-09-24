/**
 * @file
 * @date 2022/5/30
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/IAssetFactory.hpp>
#include <lstg/Core/Subsystem/Render/Camera.hpp>

namespace lstg::v2::Asset
{
    class TextureAsset;

    /**
     * 纹理资产工厂
     */
    class TextureAssetFactory :
        public Subsystem::Asset::IAssetFactory
    {
    public:  // IAssetFactory
        std::string_view GetAssetTypeName() const noexcept override;
        Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;
        Result<Subsystem::Asset::CreateAssetResult> CreateAsset(Subsystem::AssetSystem& assetSystem, Subsystem::Asset::AssetPoolPtr pool,
            std::string_view name, const nlohmann::json& arguments, Subsystem::Asset::IAssetDependencyResolver* resolver) noexcept override;

    public:
        /**
         * 调整所有 RT 大小
         * @param width 宽度
         * @param height 高度
         */
        void ResizeRenderTarget(uint32_t width, uint32_t height) noexcept;

    private:
        Result<Subsystem::Render::Camera::OutputViews> CreateViews(uint32_t width, uint32_t height) noexcept;

    private:
        // 我们在这里保存所有的 RT，当渲染大小发生变化时主动调整 RT 的大小和渲染系统对齐
        // 考虑到 RT 的数量不会很多，因此这里采取惰性释放的方式释放内存
        std::vector<std::weak_ptr<TextureAsset>> m_stRenderTargets;
    };
}
