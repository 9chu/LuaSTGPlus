/**
 * @file
 * @author 9chu
 * @date 2022/4/18
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/RenderModule.hpp>

#include <glm/ext.hpp>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/SpriteDrawing.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/TextDrawing.hpp>
#include <lstg/v2/BlendMode.hpp>
#include <lstg/v2/AssetNaming.hpp>
#include <lstg/v2/Asset/SpriteAsset.hpp>
#include <lstg/v2/Asset/HgeFontAsset.hpp>
#include <lstg/v2/Asset/TrueTypeFontAsset.hpp>
#include "detail/Helper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

LSTG_DEF_LOG_CATEGORY(RenderModule);

void RenderModule::BeginScene()
{
    // DO NOTHING
}

void RenderModule::EndScene()
{
    // DO NOTHING
}

void RenderModule::RenderClear(LSTGColor* color)
{
    detail::GetGlobalApp().GetCommandBuffer().Clear(*color);
}

void RenderModule::SetViewport(double left, double right, double bottom, double top)
{
    auto& app = detail::GetGlobalApp();
    auto bound = app.GetViewportBound();
    auto scale = bound.Width() / detail::GetGlobalApp().GetDesiredResolution().x;
    Math::ImageRectangleFloat desiredViewport = {
        static_cast<float>(bound.Left() + left * scale),
        static_cast<float>(bound.Top() + bound.Height() - top * scale),
        static_cast<float>(std::abs(right - left) * scale),
        static_cast<float>(std::abs(top - bottom) * scale)
    };
    app.GetCommandBuffer().SetViewport(
        static_cast<float>(desiredViewport.Left()),
        static_cast<float>(desiredViewport.Top()),
        static_cast<float>(desiredViewport.Width()),
        static_cast<float>(desiredViewport.Height()));
}

void RenderModule::SetOrtho(double left, double right, double bottom, double top)
{
    auto& cmdBuffer = detail::GetGlobalApp().GetCommandBuffer();
    cmdBuffer.SetView(glm::identity<glm::mat4x4>());
    cmdBuffer.SetProjection(glm::ortho<float>(static_cast<float>(left), static_cast<float>(right), static_cast<float>(bottom),
        static_cast<float>(top), 0.f, 100.f));
}

void RenderModule::SetPerspective(double eyeX, double eyeY, double eyeZ, double atX, double atY, double atZ, double upX, double upY,
    double upZ, double fovy, double aspect, double zn, double zf)
{
    auto& cmdBuffer = detail::GetGlobalApp().GetCommandBuffer();
    cmdBuffer.SetView(glm::lookAt<float, glm::defaultp>({eyeX, eyeY, eyeZ}, {atX, atY, atZ}, {upX, upY, upZ}));
    cmdBuffer.SetProjection(glm::perspective(fovy, aspect, zn, zf));
}

void RenderModule::SetFog(std::optional<double> near, std::optional<double> far, std::optional<LSTGColor*> color /* =0xFFFFFF00 (RGBA) */)
{
    auto& cmdBuffer = detail::GetGlobalApp().GetCommandBuffer();

    if (!near || !far)
    {
        cmdBuffer.SetFog(Subsystem::Render::Drawing2D::FogTypes::Disabled, {}, 0, 0);
    }
    else
    {
        auto fogNear = *near;
        auto fogFar = *far;
        auto fogColor = color ? **color : LSTGColor(255, 255, 255, 0);
        if (fogNear == -1.f)
        {
            cmdBuffer.SetFog(Subsystem::Render::Drawing2D::FogTypes::Exp, fogColor, static_cast<float>(fogFar), 0.f);
        }
        else if (fogNear == -2.f)
        {
            cmdBuffer.SetFog(Subsystem::Render::Drawing2D::FogTypes::Exp2, fogColor, static_cast<float>(fogFar), 0.f);
        }
        else
        {
            cmdBuffer.SetFog(Subsystem::Render::Drawing2D::FogTypes::Linear, fogColor, static_cast<float>(fogNear),
                static_cast<float>(fogFar));
        }
    }
}

void RenderModule::Render(LuaStack& stack, const char* imageName, double x, double y, std::optional<double> rot /* =0 */,
    std::optional<double> hscale /* =1 */, std::optional<double> vscale /* =1 */, std::optional<double> z /* =0.5 */)
{
    if (hscale && !vscale)
        vscale = hscale;

    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取精灵对象
    auto asset = assetPools->FindAsset(AssetTypes::Image, imageName);
    if (!asset)
        stack.Error("image '%s' not found.", imageName);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SpriteAsset::GetAssetTypeIdStatic());

    auto spriteAsset = static_pointer_cast<Asset::SpriteAsset>(asset);

    // 准备渲染
    auto& cmdBuffer = detail::GetGlobalApp().GetCommandBuffer();
    auto drawing = spriteAsset->GetDrawingSprite().Draw(cmdBuffer);
    if (!drawing)
    {
        LSTG_LOG_ERROR_CAT(RenderModule, "draw image '%s' fail: %s", imageName, drawing.GetError().message().c_str());
        return;
    }

    drawing->Transform(rot ? static_cast<float>(glm::radians(*rot)) : 0.f, hscale ? static_cast<float>(*hscale) : 1.f,
        vscale ? static_cast<float>(*vscale) : 1.f);
    drawing->Translate(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z ? *z : 0.5));
}

void RenderModule::RenderRect(LuaStack& stack, const char* imageName, double left, double right, double bottom, double top)
{
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取精灵对象
    auto asset = assetPools->FindAsset(AssetTypes::Image, imageName);
    if (!asset)
        stack.Error("image '%s' not found.", imageName);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SpriteAsset::GetAssetTypeIdStatic());

    auto spriteAsset = static_pointer_cast<Asset::SpriteAsset>(asset);

    // 准备渲染
    auto& cmdBuffer = detail::GetGlobalApp().GetCommandBuffer();
    auto drawing = spriteAsset->GetDrawingSprite().Draw(cmdBuffer);
    if (!drawing)
    {
        LSTG_LOG_ERROR_CAT(RenderModule, "draw image '%s' fail: %s", imageName, drawing.GetError().message().c_str());
        return;
    }

    drawing->Vertices(glm::vec2(left, top), glm::vec2(right, top), glm::vec2(right, bottom), glm::vec2(left, bottom));
    drawing->Translate(0, 0, 0.5f);
}

void RenderModule::RenderVertex(LuaStack& stack, const char* imageName, double x1, double y1, double z1, double x2, double y2, double z2,
    double x3, double y3, double z3, double x4, double y4, double z4)
{
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取精灵对象
    auto asset = assetPools->FindAsset(AssetTypes::Image, imageName);
    if (!asset)
        stack.Error("image '%s' not found.", imageName);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SpriteAsset::GetAssetTypeIdStatic());

    auto spriteAsset = static_pointer_cast<Asset::SpriteAsset>(asset);

    // 准备渲染
    auto& cmdBuffer = detail::GetGlobalApp().GetCommandBuffer();
    auto drawing = spriteAsset->GetDrawingSprite().Draw(cmdBuffer);
    if (!drawing)
    {
        LSTG_LOG_ERROR_CAT(RenderModule, "draw image '%s' fail: %s", imageName, drawing.GetError().message().c_str());
        return;
    }

    drawing->Vertices(
        glm::vec3(static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(z1)),
        glm::vec3(static_cast<float>(x2), static_cast<float>(y2), static_cast<float>(z2)),
        glm::vec3(static_cast<float>(x3), static_cast<float>(y3), static_cast<float>(z3)),
        glm::vec3(static_cast<float>(x4), static_cast<float>(y4), static_cast<float>(z4)));
}

void RenderModule::RenderTexture(LuaStack& stack, const char* textureName, const char* blend, AbsIndex vertex1, AbsIndex vertex2,
    AbsIndex vertex3, AbsIndex vertex4)
{
    auto& app = detail::GetGlobalApp();
    auto& cmdBuffer = app.GetCommandBuffer();
    auto assetPools = app.GetAssetPools();

    // 获取纹理对象
    auto asset = assetPools->FindAsset(AssetTypes::Texture, textureName);
    if (!asset)
        stack.Error("texture '%s' not found.", textureName);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::TextureAsset::GetAssetTypeIdStatic());

    auto textureAsset = static_pointer_cast<Asset::TextureAsset>(asset);
    auto tex = textureAsset->GetDrawingTexture().GetUnderlayTexture();
    auto width = textureAsset->GetWidth();
    auto height = textureAsset->GetHeight();

    // 转换 BlendMode
    v2::BlendMode blendMode(blend);

    // 收集顶点
    AbsIndex verts[4] = { vertex1, vertex2, vertex3, vertex4 };
    Subsystem::Render::Drawing2D::Vertex drawingVerts[4];
    for (int i = 0; i < 4; ++i)
    {
        auto vert = verts[i];

        lua_pushinteger(stack, 1);
        lua_gettable(stack, vert);
        auto x = static_cast<float>(lua_tonumber(stack, -1));

        lua_pushinteger(stack, 2);
        lua_gettable(stack, vert);
        auto y = static_cast<float>(lua_tonumber(stack, -1));

        lua_pushinteger(stack, 3);
        lua_gettable(stack, vert);
        auto z = static_cast<float>(lua_tonumber(stack, -1));

        lua_pushinteger(stack, 4);
        lua_gettable(stack, vert);
        auto u = static_cast<float>(lua_tonumber(stack, -1) / width);

        lua_pushinteger(stack, 5);
        lua_gettable(stack, vert);
        auto v = static_cast<float>(lua_tonumber(stack, -1) / height);

        lua_pushinteger(stack, 6);
        lua_gettable(stack, vert);
        LSTGColor* color = nullptr;
        stack.ReadValue<LSTGColor*>(-1, color);

        lua_pop(stack, 6);

        drawingVerts[i].Position = { x, y, z };
        drawingVerts[i].TexCoord = { u, v };
        if (blendMode.VertexColorBlend == v2::VertexColorBlendMode::Additive)
        {
            drawingVerts[i].Color0 = color ? color->rgba32() : 0x000000FF;
            drawingVerts[i].Color1 = 0xFFFFFFFF;
        }
        else
        {
            drawingVerts[i].Color0 = 0x000000FF;
            drawingVerts[i].Color1 = color ? color->rgba32() : 0xFFFFFFFF;
        }
    }

    // 绘制
    cmdBuffer.SetColorBlendMode(blendMode.ColorBlend);
    auto ret = cmdBuffer.DrawQuad(tex, drawingVerts);
    if (!ret)
        LSTG_LOG_ERROR_CAT(RenderModule, "draw texture '%s' fail: %s", textureName, ret.GetError().message().c_str());
}

void RenderModule::RenderText(LuaStack& stack, const char* name, const char* text, double x, double y, std::optional<double> scale /* =1 */,
    std::optional<TextAlignment> align /* =5 */)
{
    using namespace Subsystem::Render::Drawing2D;

    auto& app = detail::GetGlobalApp();
    auto& cmdBuffer = app.GetCommandBuffer();
    auto assetPools = app.GetAssetPools();

    // 获取字体对象
    auto asset = assetPools->FindAsset(AssetTypes::TexturedFont, name);
    if (!asset)
        stack.Error("textured font '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::HgeFontAsset::GetAssetTypeIdStatic());

    auto font = static_pointer_cast<Asset::HgeFontAsset>(asset);
    if (font->GetState() != Subsystem::Asset::AssetStates::Loaded)
    {
        LSTG_LOG_WARN_CAT(RenderModule, "textured font '{}' not ready", name);
        return;
    }

    const auto& blendMode = font->GetDefaultBlendMode();

    // 生成 Style
    TextDrawingStyle style;
    style.FontSize = 12;  // 对于 TexturedFont，无所谓 FontSize 填值
    style.FontScale = scale ? static_cast<float>(*scale) : 1.f;
    style.LayoutStyle.ParagraphInnerLineGap = { TextLineGapTypes::Times, 0.5f };  // 0.5倍行间距

    // 注意在此种渲染方式下，我们自己计算位置，故对于 TextDrawingStyle 总是左上角对齐
    TextHorizontalAlignment horizontalAlignment = TextHorizontalAlignment::Center;
    TextVerticalAlignment verticalAlignment = TextVerticalAlignment::Middle;
    if (align)
    {
        // 低1-2位 表示横向左中右对齐
        switch (static_cast<uint32_t>(*align) & 3)
        {
            default:
            case 0:
                horizontalAlignment = TextHorizontalAlignment::Left;
                break;
            case 1:
                horizontalAlignment = TextHorizontalAlignment::Center;
                break;
            case 2:
                horizontalAlignment = TextHorizontalAlignment::Right;
                break;
        }
        // 低3-4位
        switch ((static_cast<uint32_t>(*align) >> 2u) & 3)
        {
            default:
            case 0:
                verticalAlignment = TextVerticalAlignment::Top;
                break;
            case 1:
                verticalAlignment = TextVerticalAlignment::Middle;
                break;
            case 2:
                verticalAlignment = TextVerticalAlignment::Bottom;
                break;
        }
    }

    if (blendMode.VertexColorBlend == v2::VertexColorBlendMode::Additive)
    {
        style.AdditiveTextColor = font->GetDefaultBlendColor();
        style.MultiplyTextColor = 0xFFFFFFFF;
    }
    else
    {
        assert(blendMode.VertexColorBlend == v2::VertexColorBlendMode::Multiply);
        style.AdditiveTextColor = 0x000000FF;
        style.MultiplyTextColor = font->GetDefaultBlendColor();
    }

    // 先计算文本渲染后的大小
    auto sizeRet = TextDrawing::MeasureNonBreakSize(app.GetShapedTextCache(), font->GetFontCollection(), app.GetTextShaper(), text, style);
    if (!sizeRet)
    {
        stack.Error("font \"%s\" measure text size fail: %s", font->GetName().c_str(), sizeRet.GetError().message().c_str());
        return;
    }

    // 计算渲染位置
    Math::XYRectangle rect { static_cast<float>(x), static_cast<float>(y), sizeRet->x, sizeRet->y };
    switch (horizontalAlignment)
    {
        default:
        case TextHorizontalAlignment::Left:
            break;
        case TextHorizontalAlignment::Center:
            rect.SetLeft(rect.Left() - sizeRet->x / 2.f);
            break;
        case TextHorizontalAlignment::Right:
            rect.SetLeft(rect.Left() - sizeRet->x);
            break;
    }
    switch (verticalAlignment)
    {
        default:
        case TextVerticalAlignment::Top:
            break;
        case TextVerticalAlignment::Middle:
            rect.SetTop(rect.Top() + sizeRet->y / 2.f);
            break;
        case TextVerticalAlignment::Bottom:
            rect.SetTop(rect.Top() + sizeRet->y);
            break;
    }

    // 渲染
    cmdBuffer.SetColorBlendMode(blendMode.ColorBlend);
    auto ret = TextDrawing::Draw(app.GetShapedTextCache(), cmdBuffer, font->GetFontCollection(), app.GetFontGlyphAtlas(),
        app.GetTextShaper(), text, rect, style);
    if (!ret)
        stack.Error("font \"%s\" draw text fail: %s", font->GetName().c_str(), sizeRet.GetError().message().c_str());
}

void RenderModule::RenderTrueTypeFont(LuaStack& stack, const char* name, const char* text, double left, double right, double bottom,
    double top, int32_t fmt, LSTGColor* blend, std::optional<double> scale)
{
    using namespace Subsystem::Render::Drawing2D;

    auto& app = detail::GetGlobalApp();
    auto& cmdBuffer = app.GetCommandBuffer();
    auto assetPools = app.GetAssetPools();

    // 获取字体对象
    auto asset = assetPools->FindAsset(AssetTypes::TrueTypeFont, name);
    if (!asset)
        stack.Error("ttf font '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::TrueTypeFontAsset::GetAssetTypeIdStatic());

    auto font = static_pointer_cast<Asset::TrueTypeFontAsset>(asset);
    if (font->GetState() != Subsystem::Asset::AssetStates::Loaded)
    {
        LSTG_LOG_WARN_CAT(RenderModule, "ttf font '{}' not ready", name);
        return;
    }

    // 生成 Style
    assert(blend);
    TextDrawingStyle style;
    style.FontSize = font->GetFontSize();
    style.FontScale = (scale ? static_cast<float>(*scale) : 1.f) * 0.5f;  // 老版本渲染时给了 0.5 的缩放系数
    style.AdditiveTextColor = 0x000000FF;
    style.MultiplyTextColor = *blend;  // TTF 字体总是使用乘算
    style.LayoutStyle.ParagraphInnerLineGap = { TextLineGapTypes::Times, 0.5f };  // 0.5倍行间距

    // 由于 lstg 最早使用 GDI 渲染文本，这里需要根据 GDI 的枚举对 fmt 进行转换
    // reference: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawtext
    // from winuser.h
    enum {
        // 只考虑下述格式的支持
        DT_TOP = 0x00000000u,
        DT_LEFT = 0x00000000u,
        DT_CENTER = 0x00000001u,
        DT_RIGHT = 0x00000002u,
        DT_VCENTER = 0x00000004u,
        DT_BOTTOM = 0x00000008u,
        DT_WORDBREAK = 0x00000010u,
    };
//    if ((fmt & DT_LEFT) == DT_LEFT)
//        style.LayoutStyle.HorizontalAlignment = TextHorizontalAlignment::Left;
    if ((fmt & DT_CENTER) == DT_CENTER)
        style.LayoutStyle.HorizontalAlignment = TextHorizontalAlignment::Center;
    else if ((fmt & DT_RIGHT) == DT_RIGHT)
        style.LayoutStyle.HorizontalAlignment = TextHorizontalAlignment::Right;

//    if ((fmt & DT_TOP) == DT_TOP)
//        style.LayoutStyle.VerticalAlignment = TextVerticalAlignment::Top;
    if ((fmt & DT_VCENTER) == DT_VCENTER)
        style.LayoutStyle.VerticalAlignment = TextVerticalAlignment::Middle;
    else if ((fmt & DT_BOTTOM) == DT_BOTTOM)
        style.LayoutStyle.VerticalAlignment = TextVerticalAlignment::Bottom;

    if ((fmt & DT_WORDBREAK) == DT_WORDBREAK)
        style.LayoutStyle.LineBreak = TextLineBreakTypes::Anywhere;

    // 渲染
    Math::XYRectangle rect {
        static_cast<float>(left),
        static_cast<float>(top),
        std::max(0.f, static_cast<float>(right - left)),
        std::max(0.f, static_cast<float>(top - bottom))
    };
    cmdBuffer.SetColorBlendMode(ColorBlendMode::Alpha);
    auto ret = TextDrawing::Draw(app.GetShapedTextCache(), cmdBuffer, font->GetFontCollection(), app.GetFontGlyphAtlas(),
        app.GetTextShaper(), text, rect, style);
    if (!ret)
        stack.Error("ttf font \"%s\" draw text fail: %s", font->GetName().c_str(), ret.GetError().message().c_str());
}

void RenderModule::PushRenderTarget(LuaStack& stack, const char* name)
{
    // TODO
//    ResTexture* p = LRES.FindTexture(luaL_checkstring(L, 1));
//    if (!p)
//        return luaL_error(L, "rendertarget '%s' not found.", luaL_checkstring(L, 1));
//    if (!p->IsRenderTarget())
//        return luaL_error(L, "'%s' is a texture.", luaL_checkstring(L, 1));
//
//    if (!LAPP.PushRenderTarget(p))
//        return luaL_error(L, "push rendertarget '%s' failed.", luaL_checkstring(L, 1));
//    return 0;
}

void RenderModule::PopRenderTarget(LuaStack& stack)
{
    // TODO
//    if (!LAPP.PopRenderTarget())
//        return luaL_error(L, "pop rendertarget failed.");
//    return 0;
}

void RenderModule::PostEffect(LuaStack& stack, const char* name, const char* fx, const char* blend, std::optional<AbsIndex> args)
{
    // TODO
//    const char* texture = luaL_checkstring(L, 1);
//    const char* name = luaL_checkstring(L, 2);
//    BlendMode blend = TranslateBlendMode(L, 3);
//
//    // 获取纹理
//    ResTexture* rt = LRES.FindTexture(luaL_checkstring(L, 1));
//    if (!rt)
//        return luaL_error(L, "texture '%s' not found.", texture);
//
//    // 获取fx
//    ResFX* p = LRES.FindFX(name);
//    if (!p)
//        return luaL_error(L, "PostEffect: can't find effect '%s'.", name);
//    if (lua_istable(L, 4))
//    {
//        // 设置table上的参数到fx
//        lua_pushnil(L);  // s s t ... nil
//        while (0 != lua_next(L, 4))
//        {
//            // s s t ... nil key value
//            const char* key = luaL_checkstring(L, -2);
//            if (lua_isnumber(L, -1))
//                p->SetValue(key, (float)lua_tonumber(L, -1));
//            else if (lua_isstring(L, -1))
//            {
//                ResTexture* pTex = LRES.FindTexture(lua_tostring(L, -1));
//                if (!pTex)
//                    return luaL_error(L, "PostEffect: can't find texture '%s'.", lua_tostring(L, -1));
//                p->SetValue(key, pTex->GetTexture());
//            }
//            else if (lua_isuserdata(L, -1))
//            {
//                fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, -1, TYPENAME_COLOR));
//                p->SetValue(key, c);
//            }
//            else
//                return luaL_error(L, "PostEffect: invalid data type.");
//
//            lua_pop(L, 1);  // s s t ... nil key
//        }
//    }
//
//    if (!LAPP.PostEffect(rt, p, blend))
//        return luaL_error(L, "PostEffect failed.");
//    return 0;
}

void RenderModule::PostEffectCapture(LuaStack& stack)
{
    // TODO
//    if (!LAPP.PostEffectCapture())
//        return luaL_error(L, "PostEffectCapture failed.");
//    return 0;
}

void RenderModule::PostEffectApply(LuaStack& stack, const char* fx, const char* blend, std::optional<AbsIndex> args)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//    BlendMode blend = TranslateBlendMode(L, 2);
//
//    // 获取fx
//    ResFX* p = LRES.FindFX(name);
//    if (!p)
//        return luaL_error(L, "PostEffectApply: can't find effect '%s'.", name);
//    if (lua_istable(L, 3))
//    {
//        // 设置table上的参数到fx
//        lua_pushnil(L);  // s s t ... nil
//        while (0 != lua_next(L, 3))
//        {
//            // s s t ... nil key value
//            const char* key = luaL_checkstring(L, -2);
//            if (lua_isnumber(L, -1))
//                p->SetValue(key, (float)lua_tonumber(L, -1));
//            else if (lua_isstring(L, -1))
//            {
//                ResTexture* pTex = LRES.FindTexture(lua_tostring(L, -1));
//                if (!pTex)
//                    return luaL_error(L, "PostEffectApply: can't find texture '%s'.", lua_tostring(L, -1));
//                p->SetValue(key, pTex->GetTexture());
//            }
//            else if (lua_isuserdata(L, -1))
//            {
//                fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, -1, TYPENAME_COLOR));
//                p->SetValue(key, c);
//            }
//            else
//                return luaL_error(L, "PostEffectApply: invalid data type.");
//
//            lua_pop(L, 1);  // s s t ... nil key
//        }
//    }
//
//    if (!LAPP.PostEffectApply(p, blend))
//        return luaL_error(L, "PostEffectApply failed.");
//    return 0;
}
