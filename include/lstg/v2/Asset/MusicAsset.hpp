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
    class MusicAssetLoader;

    /**
     * 音乐循环节
     * 使用<起始时间,终止时间>表示循环节。
     * 毫秒为单位。
     * 负数表示以音频结尾为起点。
     */
    using MusicLoopRange = std::tuple<int32_t, int32_t>;

    /**
     * 音乐资源
     */
    class MusicAsset :
        public Subsystem::Asset::Asset
    {
        friend class MusicAssetLoader;

    public:
        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        MusicAsset(std::string name, std::string path, MusicLoopRange loopRange);

    public:
        /**
         * 获取路径
         */
        [[nodiscard]] const std::string& GetPath() const noexcept { return m_stPath; }

        /**
         * 获取音频数据
         */
        [[nodiscard]] const Subsystem::Audio::SoundDataPtr& GetSoundData() const noexcept { return m_pSoundData; }

        /**
         * 获取循环节
         */
        [[nodiscard]] MusicLoopRange GetLoopRange() const noexcept { return m_stMusicLoopRange; }

        /**
         * 设置循环节
         */
        void SetLoopRange(MusicLoopRange loopRange) noexcept { m_stMusicLoopRange = loopRange; }

    protected:  // Asset
        [[nodiscard]] Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;

    private:
        void UpdateResource(Subsystem::Audio::SoundDataPtr data) noexcept;

    private:
        const std::string m_stPath;
        Subsystem::Audio::SoundDataPtr m_pSoundData;
        MusicLoopRange m_stMusicLoopRange;
    };

    using MusicAssetPtr = std::shared_ptr<MusicAsset>;
}
