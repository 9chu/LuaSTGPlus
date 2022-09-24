/**
 * @file
 * @date 2022/6/28
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <list>
#include <memory>
#include <vector>
#include <unordered_map>
#include "../../../Math/Rectangle.hpp"
#include "../Texture.hpp"
#include "../Texture2DData.hpp"
#include "FontGlyphRasterParam.hpp"

namespace lstg::Subsystem::Render::Font
{
    class IFontFace;

    namespace detail
    {
        struct AtlasSlotStrip;
        struct AtlasTexture;

        struct AtlasSlotKey
        {
            IFontFace* Face = nullptr;
            FontGlyphRasterParam Param;
            FontGlyphId GlyphId = 0;

            bool operator==(const AtlasSlotKey& rhs) const noexcept
            {
                return Face == rhs.Face && Param == rhs.Param && GlyphId == rhs.GlyphId;
            }

            bool operator!=(const AtlasSlotKey& rhs) const noexcept
            {
                return !operator==(rhs);
            }
        };

        struct AtlasSlot
        {
            AtlasSlotStrip* Parent = nullptr;
            AtlasSlot* Prev = nullptr;
            AtlasSlot* Next = nullptr;
            uint32_t SlotLeft = 0;  ///< @brief 槽距离纹理左边的像素数
            uint32_t SlotWidth = 0;  ///< @brief 槽的宽度（不含 Margin）
            uint32_t SlotHeight = 0;  ///< @brief 槽的高度（不含 Margin），总是 <= StripHeight

            // 字形信息
            std::weak_ptr<IFontFace> Face;  ///< @brief 关联的字体
            Math::UVRectangle TextureRect;  ///< @brief UV矩形
            glm::vec2 DrawOffset;  ///< @brief 笔触偏移（像素）
        };

        struct AtlasSlotStrip
        {
            AtlasTexture* Parent = nullptr;
            uint32_t StripTop = 0;  ///< @brief 距离顶边的距离（像素）
            uint32_t StripHeight = 0;  ///< @brief 条带的像素高度（不含边界）
            AtlasSlot* SlotListHead = nullptr;   ///< @brief 链表头
        };

        /**
         * 图集纹理
         * 一张纹理被划分为若干 Slot 条带，每个条带上有若干 Slot，一个 Slot 用于存储一张字形的 Bitmap。
         * 每一个 Slot 条带存储相同高度的 Slot。
         * Slot 之间、Slot 条带之间存在 SlotMargin 个像素的间距。
         */
        struct AtlasTexture
        {
            TexturePtr Texture;
            Texture2DData TextureData;
            std::list<AtlasSlotStrip> SlotRowHeaders;
            Math::ImageRectangle ImageDirtyRegion;

            AtlasTexture(RenderSystem& renderSystem, uint32_t width, uint32_t height);
        };
    }
}

template <>
struct std::hash<lstg::Subsystem::Render::Font::detail::AtlasSlotKey>
{
    std::size_t operator()(const lstg::Subsystem::Render::Font::detail::AtlasSlotKey& v) const noexcept
    {
        using namespace lstg::Subsystem::Render::Font;

        auto hash = std::hash<IFontFace*>{}(v.Face);
        hash ^= std::hash<FontGlyphRasterParam>{}(v.Param);
        hash ^= std::hash<FontGlyphId>{}(v.GlyphId);
        return hash;
    }
};

namespace lstg::Subsystem::Render::Font
{
    /**
     * 字体字形图集信息
     */
    struct FontGlyphAtlasInfo
    {
        TexturePtr Texture;  ///< @brief 关联纹理
        Math::UVRectangle TextureRect;  ///< @brief UV矩形
        glm::vec2 DrawOffset;  ///< @brief 笔触偏移（像素），X 表示笔触起点到位图左边的距离，Y 表示笔触起点距离位图顶边的距离（Y轴向上）
        glm::vec2 DrawSize;  ///< @brief 大小
    };

    /**
     * 位图数据源
     */
    struct BitmapSource
    {
        const uint8_t* Buffer = nullptr;
        uint32_t Width = 0;  ///< @brief 宽，像素数
        uint32_t Height = 0;  ///< @brief 高，像素数
        uint32_t Stride = 0;  ///< @brief 行字节数
        int32_t DrawLeftOffset = 0;  ///< @brief 笔触偏移
        int32_t DrawTopOffset = 0;  ///< @brief 笔触偏移
    };

    /**
     * 动态字形图集
     */
    class DynamicFontGlyphAtlas
    {
        using LookupTableContainer = std::unordered_map<detail::AtlasSlotKey, detail::AtlasSlot*>;

    public:
        DynamicFontGlyphAtlas(RenderSystem& renderSystem) noexcept;
        ~DynamicFontGlyphAtlas();

    public:
        /**
         * 获取图集个数
         */
        size_t GetAtlasCount() const noexcept { return m_stAtlasList.size(); }

        /**
         * 获取图集关联的纹理
         * @note 调试用
         * @param index 索引
         * @return 纹理，或者 nullptr
         */
        TexturePtr GetAtlasTexture(size_t index) const noexcept;

        /**
         * 查找字形
         * @param out 图集查找结果
         * @param fontFace 关联的字体
         * @param param 光栅化参数
         * @param glyphId 字形ID
         * @return 是否存在字形
         */
        bool FindGlyph(FontGlyphAtlasInfo& out, IFontFace* fontFace, FontGlyphRasterParam param, FontGlyphId glyphId) const noexcept;

        /**
         * 缓存字形位图（BGRA）
         * @param fontFace 关联的字体
         * @param param 光栅化参数
         * @param glyphId 字形ID
         * @param source 位图数据源
         * @return 是否成功，成功返回图集信息
         */
        Result<FontGlyphAtlasInfo> CacheGlyphFromBGRA(std::shared_ptr<IFontFace> fontFace, FontGlyphRasterParam param, FontGlyphId glyphId,
            BitmapSource source) noexcept;

        /**
         * 缓存字形位图（灰阶）
         * @param fontFace 关联的字体
         * @param param 光栅化参数
         * @param glyphId 字形ID
         * @param source 位图数据源
         * @param grays 灰阶数量
         * @return 是否成功，成功返回图集信息
         */
        Result<FontGlyphAtlasInfo> CacheGlyphFromGrayscale(std::shared_ptr<IFontFace> fontFace, FontGlyphRasterParam param,
            FontGlyphId glyphId, BitmapSource source, uint32_t grays) noexcept;

        /**
         * 重置
         */
        void Reset() noexcept;

        /**
         * 提交状态
         */
        Result<void> Commit() noexcept;

    private:
        /**
         * 寻找合适的 Slot
         * @param atlasTexture 纹理
         * @param width 宽度（不含边界）
         * @param height 高度（不含边界）
         * @param stripHeight 高度（指 Strip 高度）
         * @return 槽
         */
        Result<detail::AtlasSlot*> AllocSlotInTexture(detail::AtlasTexture& atlasTexture, uint32_t width, uint32_t height,
            uint32_t stripHeight) noexcept;

        /**
         * 分配一个槽
         * @param key 查找 Key
         * @param source Bitmap 信息
         * @return 槽的迭代器
         */
        Result<LookupTableContainer::iterator> AllocSlot(const detail::AtlasSlotKey& key, const BitmapSource& source) noexcept;

        /**
         * 删除一个槽
         * @param it 迭代器
         * @return 下一个迭代器
         */
        LookupTableContainer::iterator DeleteSlot(LookupTableContainer::iterator it) noexcept;

        /**
         * 删除一个槽
         * @param slot 槽
         */
        void DeleteSlot(detail::AtlasSlot* slot) noexcept;

        /**
         * 删除不用的槽
         * @return 删除的个数
         */
        size_t DeleteUnused() noexcept;

        /**
         * 缓存字形位图
         * @param fontFace 关联的字体
         * @param param 光栅化参数
         * @param glyphId 字形ID
         * @param source 位图数据源
         * @param isGrayscale 是否是灰阶图
         * @param grays 灰阶数量
         * @return 是否成功，成功返回图集信息
         */
        Result<FontGlyphAtlasInfo> CacheGlyphFrom(std::shared_ptr<IFontFace> fontFace, FontGlyphRasterParam param, FontGlyphId glyphId,
            BitmapSource source, bool isGrayscale, uint32_t grays) noexcept;

    private:
        RenderSystem& m_stRenderSystem;
        std::list<detail::AtlasTexture> m_stAtlasList;
        LookupTableContainer m_stLookupTable;
    };

    using DynamicFontGlyphAtlasPtr = std::shared_ptr<DynamicFontGlyphAtlas>;
}
