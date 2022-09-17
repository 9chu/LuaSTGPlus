/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Audio/ISoundData.hpp>
#include <lstg/Core/Subsystem/Asset/Asset.hpp>

namespace lstg::v2::Asset
{
    class SoundAssetLoader;

    /**
     * 音频资源
     */
    class SoundAsset :
        public Subsystem::Asset::Asset
    {
        friend class SoundAssetLoader;

    public:
        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        SoundAsset(std::string name, std::string path);

    public:
        /**
         * 获取路径
         */
        [[nodiscard]] const std::string& GetPath() const noexcept { return m_stPath; }

        /**
         * 获取音频数据
         */
        [[nodiscard]] const Subsystem::Audio::SoundDataPtr& GetSoundData() const noexcept { return m_pSoundData; }

    protected:  // Asset
        [[nodiscard]] Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;

    private:
        void UpdateResource(Subsystem::Audio::SoundDataPtr data) noexcept;

    private:
        const std::string m_stPath;
        Subsystem::Audio::SoundDataPtr m_pSoundData;
    };

    using SoundAssetPtr = std::shared_ptr<SoundAsset>;
}
