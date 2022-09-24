/**
 * @file
 * @author 9chu
 * @date 2022/4/18
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Bridge/RenderModule.hpp>

#include <glm/ext.hpp>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/RenderSystem.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/SpriteDrawing.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/TextDrawing.hpp>
#include <lstg/v2/BlendMode.hpp>
#include <lstg/v2/AssetNaming.hpp>
#include <lstg/v2/Asset/SpriteAsset.hpp>
#include <lstg/v2/Asset/HgeFontAsset.hpp>
#include <lstg/v2/Asset/TrueTypeFontAsset.hpp>
#include <lstg/v2/Asset/EffectAsset.hpp>
#include "detail/Helper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

LSTG_DEF_LOG_CATEGORY(RenderModule);

void RenderModule::BeginScene()
{
    LSTG_LOG_DEPRECATED(RenderModule, BeginScene);
}

void RenderModule::EndScene()
{
    LSTG_LOG_DEPRECATED(RenderModule, EndScene);
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
        stack.Error("font \"%s\" draw text fail: %s", font->GetName().c_str(), ret.GetError().message().c_str());
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
    auto& app = detail::GetGlobalApp();
    auto assetPools = app.GetAssetPools();

    // 获取纹理对象
    auto asset = assetPools->FindAsset(AssetTypes::Texture, name);
    if (!asset)
        stack.Error("render target '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::TextureAsset::GetAssetTypeIdStatic());

    // 检查是否是 RT
    auto texture = static_pointer_cast<Asset::TextureAsset>(asset);
    if (!texture->IsRenderTarget())
        stack.Error("'%s' is not a render target.", name);

    // 放入堆栈
    if (!app.PushRenderTarget(texture->GetDrawingTexture().GetUnderlayTexture()))
        stack.Error("push render target '%s' failed.", name);
}

void RenderModule::PopRenderTarget(LuaStack& stack)
{
    auto& app = detail::GetGlobalApp();

    // 从堆栈弹出
    if (!app.PopRenderTarget())
        stack.Error("pop render target failed.");
}

namespace
{
    void PostEffectImpl(Subsystem::Script::LuaStack& stack, Subsystem::Render::TexturePtr tex, const char* fx, const char* blend,
        std::optional<Subsystem::Script::LuaStack::AbsIndex> args)
    {
        auto& app = v2::Bridge::detail::GetGlobalApp();
        auto& cmdBuffer = app.GetCommandBuffer();
        auto assetPools = app.GetAssetPools();

        // 获取效果对象
        auto effectAsset = assetPools->FindAsset(v2::AssetTypes::Effect, fx);
        if (!effectAsset)
            stack.Error("effect '%s' not found.", fx);
        assert(effectAsset);
        assert(effectAsset->GetAssetTypeId() == v2::Asset::EffectAsset::GetAssetTypeIdStatic());
        auto effect = static_pointer_cast<v2::Asset::EffectAsset>(effectAsset);
        if (effect->GetState() != Subsystem::Asset::AssetStates::Loaded)
        {
            LSTG_LOG_WARN_CAT(RenderModule, "Effect '{}' is not loaded", fx);
            return;
        }

        // 获取混合模式
        v2::BlendMode blendMode(blend);

        // 设置参数
        if (args)
        {
            const auto& mat = effect->GetDefaultMaterial();

            luaL_checktype(stack, *args, LUA_TTABLE);
            lua_pushnil(stack);  // s s s t ... nil
            while (0 != lua_next(stack, *args))
            {
                // s s s t ... nil key value
                const char* key = luaL_checkstring(stack, -2);
                if (lua_isnumber(stack, -1))
                {
                    auto ret = mat->SetUniform<float>(key, static_cast<float>(lua_tonumber(stack, -1)));
                    if (!ret && ret.GetError() == make_error_code(Subsystem::Render::GraphDef::DefinitionError::SymbolTypeMismatched))
                    {
                        // 试图用 int32_t
                        ret = mat->SetUniform<int32_t>(key, static_cast<int32_t>(lua_tointeger(stack, -1)));
                    }
                    if (!ret)
                        stack.Error("Set uniform '%s' error: %s", key, ret.GetError().message().c_str());
                }
                else if (lua_isstring(stack, -1))  // 字符串类型视为纹理
                {
                    auto texAsset = assetPools->FindAsset(v2::AssetTypes::Texture, lua_tostring(stack, -1));
                    if (!texAsset)
                        stack.Error("texture '%s' not found.", lua_tostring(stack, -1));
                    assert(texAsset);
                    assert(texAsset->GetAssetTypeId() == v2::Asset::TextureAsset::GetAssetTypeIdStatic());
                    auto textureToSet = static_pointer_cast<v2::Asset::TextureAsset>(texAsset)->GetDrawingTexture().GetUnderlayTexture();
                    if (!textureToSet)
                    {
                        // 如果纹理没有加载，使用默认纹理替代
                        textureToSet = app.GetSubsystem<Subsystem::RenderSystem>()->GetDefaultTexture2D();
                    }
                    assert(textureToSet);

                    auto ret = mat->SetTexture(key, textureToSet);
                    if (!ret)
                        stack.Error("Set texture '%s' error: %s", key, ret.GetError().message().c_str());
                }
                else if (lua_isuserdata(stack, -1))  // 颜色类视作 uint32_t
                {
                    auto c = stack.ReadValue<LSTGColor*>(-1);
                    if (!c)
                        stack.Error("unexpected userdata, LSTGColor expected at argument '%s'", key);

                    auto ret = mat->SetUniform<uint32_t>(key, c->rgba32());
                    if (!ret)
                        stack.Error("Set uniform '%s' error: %s", key, ret.GetError().message().c_str());
                }
                else
                {
                    stack.Error("invalid data type at argument '%s'", key);
                }

                lua_pop(stack, 1);  // s s s t ... nil key
            }
        }

        // 执行绘制过程
        static const Subsystem::Render::Drawing2D::Vertex kQuadVertics[4] = {
            { { -1.f,  1.f, 0.5f }, { 0.f, 0.f }, 0x000000FF, 0xFFFFFFFF },
            { {  1.f,  1.f, 0.5f }, { 1.f, 0.f }, 0x000000FF, 0xFFFFFFFF },
            { {  1.f, -1.f, 0.5f }, { 1.f, 1.f }, 0x000000FF, 0xFFFFFFFF },
            { { -1.f, -1.f, 0.5f }, { 0.f, 1.f }, 0x000000FF, 0xFFFFFFFF },
        };
        auto mat = cmdBuffer.GetMaterial();  // 保存 Material 供恢复

        cmdBuffer.SetColorBlendMode(blendMode.ColorBlend);
        cmdBuffer.SetMaterial(effect->GetDefaultMaterial());

        // PostEffect 渲染过程中不会改变其他的渲染状态，例如投影矩阵等，交由 Shader 自行处理以节省开销
        auto ret = cmdBuffer.DrawQuad(std::move(tex), kQuadVertics);

        cmdBuffer.SetMaterial(std::move(mat));  // 恢复 Material
        if (!ret)
            stack.Error("DrawQuad error: %s", ret.GetError().message().c_str());
    }
}

void RenderModule::PostEffect(LuaStack& stack, const char* name, const char* fx, const char* blend, std::optional<AbsIndex> args)
{
    auto& app = detail::GetGlobalApp();
    auto assetPools = app.GetAssetPools();

    // 获取纹理对象
    auto asset = assetPools->FindAsset(AssetTypes::Texture, name);
    if (!asset)
        stack.Error("texture '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::TextureAsset::GetAssetTypeIdStatic());
    auto texture = static_pointer_cast<Asset::TextureAsset>(asset);

    PostEffectImpl(stack, texture->GetDrawingTexture().GetUnderlayTexture(), fx, blend, args);
}

void RenderModule::PostEffectCapture(LuaStack& stack)
{
    auto& app = detail::GetGlobalApp();

    auto defaultRT = app.GetDefaultRenderTarget();
    if (!defaultRT)
        stack.Error("Get default rt error: %s", defaultRT.GetError().message().c_str());

    assert(defaultRT);
    auto ret = app.PushRenderTarget(std::move(*defaultRT));
    if (!ret)
        stack.Error("Push rt error: %s", ret.GetError().message().c_str());
}

void RenderModule::PostEffectApply(LuaStack& stack, const char* fx, const char* blend, std::optional<AbsIndex> args)
{
    auto& app = detail::GetGlobalApp();

    auto defaultRT = app.GetDefaultRenderTarget();
    if (!defaultRT)
        stack.Error("Get default rt error: %s", defaultRT.GetError().message().c_str());

    auto lastRT = app.PopRenderTarget();
    if (!lastRT)
        stack.Error("Pop rt error: %s", lastRT.GetError().message().c_str());

    assert(lastRT);
    if (*lastRT != *defaultRT)
        stack.Error("Unexpected rt on top of the rt stack");

    PostEffectImpl(stack, std::move(*lastRT), fx, blend, args);
}
