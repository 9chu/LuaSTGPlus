/**
 * @file
 * @date 2022/6/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <array>
#include "CommandBuffer.hpp"

namespace lstg::Subsystem::Render::Drawing2D
{
    /**
     * 精灵绘制工具
     */
    class SpriteDrawing
    {
    public:
        static Result<SpriteDrawing> Draw(CommandBuffer& cmdBuffer, TexturePtr tex2d) noexcept
        {
            auto ret = cmdBuffer.DrawQuadInPlace(std::move(tex2d));
            if (!ret)
                return ret.GetError();

            // 使用空数据初始化
            auto dest = *ret;
            assert(dest.GetSize() == 4);
            ::memset(dest.GetData(), 0, sizeof(Vertex) * 4);
            dest[0].Color0 = 0x000000FF;
            dest[0].Color1 = 0xFFFFFFFF;
            dest[1].Color0 = 0x000000FF;
            dest[1].Color1 = 0xFFFFFFFF;
            dest[2].Color0 = 0x000000FF;
            dest[2].Color1 = 0xFFFFFFFF;
            dest[3].Color0 = 0x000000FF;
            dest[3].Color1 = 0xFFFFFFFF;

            return SpriteDrawing(dest);
        }

        static Result<SpriteDrawing> Draw(CommandBuffer& cmdBuffer, TexturePtr tex2d, const std::array<Vertex, 4>& org) noexcept
        {
            auto ret = cmdBuffer.DrawQuadInPlace(std::move(tex2d));
            if (!ret)
                return ret.GetError();

            // 从传入的顶点初始化
            auto dest = *ret;
            assert(dest.GetSize() == 4);
            ::memcpy(dest.GetData(), org.data(), sizeof(Vertex) * 4);

            return SpriteDrawing( dest);
        }

    protected:
        explicit SpriteDrawing(Span<Vertex> dest)
            : m_stVertexList(dest)
        {
            assert(dest.GetSize() == 4);
        }

    public:
        ~SpriteDrawing() = default;

    public:
        /**
         * 设置顶点坐标
         * @note 顶点按照顺时针排布
         * @param v1 顶点1
         * @param v2 顶点2
         * @param v3 顶点3
         * @param v4 顶点4
         */
        SpriteDrawing& Vertices(glm::vec2 v1, glm::vec2 v2, glm::vec2 v3, glm::vec2 v4)
        {
            m_stVertexList[0].Position.x = v1.x;
            m_stVertexList[0].Position.y = v1.y;
            m_stVertexList[1].Position.x = v2.x;
            m_stVertexList[1].Position.y = v2.y;
            m_stVertexList[2].Position.x = v3.x;
            m_stVertexList[2].Position.y = v3.y;
            m_stVertexList[3].Position.x = v4.x;
            m_stVertexList[3].Position.y = v4.y;
            return *this;
        }
        SpriteDrawing& Vertices(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4)
        {
            m_stVertexList[0].Position = v1;
            m_stVertexList[1].Position = v2;
            m_stVertexList[2].Position = v3;
            m_stVertexList[3].Position = v4;
            return *this;
        }

        /**
         * 形状
         * @note 坐标系为 Y 向上，X 向右
         * @param w 宽度
         * @param h 高度
         * @param cx 中心X，相对于左边
         * @param cy 中心Y，相对于顶边
         */
        SpriteDrawing& Shape(float w, float h, float cx, float cy) noexcept
        {
            m_stVertexList[0].Position = { -cx, cy, 0.f };
            m_stVertexList[1].Position = { w - cx, cy, 0.f };
            m_stVertexList[2].Position = { w - cx, cy - h, 0.f };
            m_stVertexList[3].Position = { -cx, cy - h, 0.f };
            return *this;
        }

        /**
         * 变换
         * @param rot 旋转，弧度
         * @param scaleX 缩放X
         * @param scaleY 缩放Y
         */
        SpriteDrawing& Transform(float rot, float scaleX=1.f, float scaleY=1.f) noexcept
        {
            // Scale
            if (scaleX != 1.f)
            {
                m_stVertexList[0].Position.x *= scaleX;
                m_stVertexList[1].Position.x *= scaleX;
                m_stVertexList[2].Position.x *= scaleX;
                m_stVertexList[3].Position.x *= scaleX;
            }
            if (scaleY != 1.f)
            {
                m_stVertexList[0].Position.y *= scaleY;
                m_stVertexList[1].Position.y *= scaleY;
                m_stVertexList[2].Position.y *= scaleY;
                m_stVertexList[3].Position.y *= scaleY;
            }

            // Rotation
            if (rot != 0.f)
            {
                auto sinR = ::sin(rot);
                auto cosR = ::cos(rot);

#define LSTG_ROT_SINCOS(X, Y) \
                do { \
                    auto tx = (X) * cosR - (Y) * sinR; \
                    auto ty = (X) * sinR + (Y) * cosR; \
                    (X) = tx; (Y) = ty; \
                } while (false)

                LSTG_ROT_SINCOS(m_stVertexList[0].Position.x, m_stVertexList[0].Position.y);
                LSTG_ROT_SINCOS(m_stVertexList[1].Position.x, m_stVertexList[1].Position.y);
                LSTG_ROT_SINCOS(m_stVertexList[2].Position.x, m_stVertexList[2].Position.y);
                LSTG_ROT_SINCOS(m_stVertexList[3].Position.x, m_stVertexList[3].Position.y);
#undef LSTG_ROT_SINCOS
            }

            return *this;
        }

        /**
         * 设置纹理
         * @param u U坐标
         * @param v V坐标
         * @param w 宽度
         * @param h 高度
         */
        SpriteDrawing& Texture(float u=0.f, float v=0.f, float w=1.f, float h=1.f) noexcept
        {
            m_stVertexList[0].TexCoord = { u, v };
            m_stVertexList[1].TexCoord = { u + w, v };
            m_stVertexList[2].TexCoord = { u + w, v + h };
            m_stVertexList[3].TexCoord = { u, v + h };
            return *this;
        }

        /**
         * 设置加算颜色
         * @param color 颜色
         */
        SpriteDrawing& SetAdditiveColor(const std::array<ColorRGBA32, 4>& color) noexcept
        {
            m_stVertexList[0].Color0 = color[0];
            m_stVertexList[1].Color0 = color[1];
            m_stVertexList[2].Color0 = color[2];
            m_stVertexList[3].Color0 = color[3];
            return *this;
        }
        SpriteDrawing& SetAdditiveColor(ColorRGBA32 color) noexcept
        {
            m_stVertexList[0].Color0 = color;
            m_stVertexList[1].Color0 = color;
            m_stVertexList[2].Color0 = color;
            m_stVertexList[3].Color0 = color;
            return *this;
        }

        /**
         * 设置乘算颜色
         * @param color 颜色
         */
        SpriteDrawing& SetMultiplyColor(const std::array<ColorRGBA32, 4>& color) noexcept
        {
            m_stVertexList[0].Color1 = color[0];
            m_stVertexList[1].Color1 = color[1];
            m_stVertexList[2].Color1 = color[2];
            m_stVertexList[3].Color1 = color[3];
            return *this;
        }
        SpriteDrawing& SetMultiplyColor(ColorRGBA32 color) noexcept
        {
            m_stVertexList[0].Color1 = color;
            m_stVertexList[1].Color1 = color;
            m_stVertexList[2].Color1 = color;
            m_stVertexList[3].Color1 = color;
            return *this;
        }

        /**
         * 平移
         * @note 坐标系为 Y 向上，X 向右
         * @param offsetX 坐标X
         * @param offsetY 坐标Y
         * @param offsetZ 坐标Z
         */
        SpriteDrawing& Translate(float offsetX, float offsetY, float offsetZ) noexcept
        {
            // Translate
            if (offsetX != 0.f)
            {
                m_stVertexList[0].Position.x += offsetX;
                m_stVertexList[1].Position.x += offsetX;
                m_stVertexList[2].Position.x += offsetX;
                m_stVertexList[3].Position.x += offsetX;
            }
            if (offsetY != 0.f)
            {
                m_stVertexList[0].Position.y += offsetY;
                m_stVertexList[1].Position.y += offsetY;
                m_stVertexList[2].Position.y += offsetY;
                m_stVertexList[3].Position.y += offsetY;
            }
            if (offsetZ != 0.f)
            {
                m_stVertexList[0].Position.z += offsetZ;
                m_stVertexList[1].Position.z += offsetZ;
                m_stVertexList[2].Position.z += offsetZ;
                m_stVertexList[3].Position.z += offsetZ;
            }
            return *this;
        }

    private:
        Span<Vertex> m_stVertexList;
    };
}
