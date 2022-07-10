/**
 * @file
 * @date 2022/7/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "../Texture.hpp"

namespace lstg::Subsystem::Render::Drawing2D
{
    /**
     * 2D 纹理
     */
    class Texture2D
    {
    public:
        /**
         * 获取纹理对象
         */
        const TexturePtr& GetUnderlayTexture() const noexcept;

        /**
         * 设置纹理对象
         * @param tex 纹理
         */
        void SetUnderlayTexture(TexturePtr tex) noexcept;

        /**
         * 获取 PPU
         */
        [[nodiscard]] float GetPixelPerUnit() const noexcept;

        /**
         * 设置 PPU
         * @param ppu Pixel Per Unit
         */
        void SetPixelPerUnit(float ppu) noexcept;

        /**
         * 获得宽度
         * = 实际像素宽度 / PPU
         * @note 当纹理为 nullptr 时，返回默认空白纹理的大小
         */
        [[nodiscard]] float GetWidth() const noexcept;

        /**
         * 获取高度
         * = 实际像素高度 / PPU
         * @note 当纹理为 nullptr 时，返回默认空白纹理的大小
         */
        [[nodiscard]] float GetHeight() const noexcept;

    private:
        TexturePtr m_pTexture;
        float m_fPPU = 1.f;
    };
}
