/**
 * @file
 * @author 9chu
 * @date 2022/4/18
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/RenderModule.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

void RenderModule::BeginScene()
{
    // TODO
//    if (!LAPP.BeginScene())
//        return luaL_error(L, "can't invoke 'BeginScene'.");
//    return 0;
}

void RenderModule::EndScene()
{
    // TODO
//    if (!LAPP.EndScene())
//        return luaL_error(L, "can't invoke 'EndScene'.");
//    return 0;
}

void RenderModule::RenderClear(LSTGColor* color)
{
    // TODO
//    fcyColor* c = static_cast<fcyColor*>(luaL_checkudata(L, 1, TYPENAME_COLOR));
//    LAPP.ClearScreen(*c);
//    return 0;
}

void RenderModule::SetViewport(double left, double right, double bottom, double top)
{
    // TODO
//    if (!LAPP.SetViewport(
//        luaL_checknumber(L, 1),
//        luaL_checknumber(L, 2),
//        luaL_checknumber(L, 3),
//        luaL_checknumber(L, 4)
//    ))
//    {
//        return luaL_error(L, "invalid arguments for 'SetViewport'.");
//    }
//    return 0;
}

void RenderModule::SetOrtho(double left, double right, double bottom, double top)
{
    // TODO
//    LAPP.SetOrtho(
//        static_cast<float>(luaL_checknumber(L, 1)),
//        static_cast<float>(luaL_checknumber(L, 2)),
//        static_cast<float>(luaL_checknumber(L, 3)),
//        static_cast<float>(luaL_checknumber(L, 4))
//    );
//    return 0;
}

void RenderModule::SetPerspective(double eyeX, double eyeY, double eyeZ, double atX, double atY, double atZ, double upX, double upY,
    double upZ, double fovy, double aspect, double zn, double zf)
{
    // TODO
//    LAPP.SetPerspective(
//        static_cast<float>(luaL_checknumber(L, 1)),
//        static_cast<float>(luaL_checknumber(L, 2)),
//        static_cast<float>(luaL_checknumber(L, 3)),
//        static_cast<float>(luaL_checknumber(L, 4)),
//        static_cast<float>(luaL_checknumber(L, 5)),
//        static_cast<float>(luaL_checknumber(L, 6)),
//        static_cast<float>(luaL_checknumber(L, 7)),
//        static_cast<float>(luaL_checknumber(L, 8)),
//        static_cast<float>(luaL_checknumber(L, 9)),
//        static_cast<float>(luaL_checknumber(L, 10)),
//        static_cast<float>(luaL_checknumber(L, 11)),
//        static_cast<float>(luaL_checknumber(L, 12)),
//        static_cast<float>(luaL_checknumber(L, 13))
//    );
//    return 0;
}

void RenderModule::SetFog(std::optional<double> near, std::optional<double> far, std::optional<LSTGColor*> color /* =0xFFFFFF00 (RGBA) */)
{
    // TODO
//    if (lua_gettop(L) == 3)
//        LAPP.SetFog(
//            static_cast<float>(luaL_checknumber(L, 1)),
//            static_cast<float>(luaL_checknumber(L, 2)),
//            *(static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR)))
//        );
//    else if (lua_gettop(L) == 2)
//        LAPP.SetFog(
//            static_cast<float>(luaL_checknumber(L, 1)),
//            static_cast<float>(luaL_checknumber(L, 2)),
//            0xFF000000
//        );
//    else
//        LAPP.SetFog(0.0f, 0.0f, 0x00FFFFFF);
//    return 0;
}

void RenderModule::Render(const char* imageName, double x, double y, std::optional<double> rot /* =0 */,
    std::optional<double> hscale /* =1 */, std::optional<double> vscale /* =1 */, std::optional<double> z /* =0.5 */)
{
    // TODO
//    if (!LAPP.Render(
//        luaL_checkstring(L, 1),
//        static_cast<float>(luaL_checknumber(L, 2)),
//        static_cast<float>(luaL_checknumber(L, 3)),
//        static_cast<float>(luaL_optnumber(L, 4, 0.) * LDEGREE2RAD),
//        static_cast<float>(luaL_optnumber(L, 5, 1.) * LRES.GetGlobalImageScaleFactor()),
//        static_cast<float>(luaL_optnumber(L, 6, luaL_optnumber(L, 5, 1.)) * LRES.GetGlobalImageScaleFactor()),
//        static_cast<float>(luaL_optnumber(L, 7, 0.5))
//    ))
//    {
//        return luaL_error(L, "can't render '%m'", luaL_checkstring(L, 1));
//    }
//    return 0;
}

void RenderModule::RenderRect(const char* imageName, double left, double right, double bottom, double top)
{
    // TODO
//    if (!LAPP.RenderRect(
//        luaL_checkstring(L, 1),
//        static_cast<float>(luaL_checknumber(L, 2)),
//        static_cast<float>(luaL_checknumber(L, 5)),
//        static_cast<float>(luaL_checknumber(L, 3)),
//        static_cast<float>(luaL_checknumber(L, 4))
//    ))
//    {
//        return luaL_error(L, "can't render '%m'", luaL_checkstring(L, 1));
//    }
//    return 0;
}

void RenderModule::RenderVertex(const char* imageName, double x1, double y1, double z1, double x2, double y2, double z2, double x3,
    double y3, double z3, double x4, double y4, double z4)
{
    // TODO
//    if (!LAPP.Render4V(
//        luaL_checkstring(L, 1),
//        static_cast<float>(luaL_checknumber(L, 2)),
//        static_cast<float>(luaL_checknumber(L, 3)),
//        static_cast<float>(luaL_checknumber(L, 4)),
//        static_cast<float>(luaL_checknumber(L, 5)),
//        static_cast<float>(luaL_checknumber(L, 6)),
//        static_cast<float>(luaL_checknumber(L, 7)),
//        static_cast<float>(luaL_checknumber(L, 8)),
//        static_cast<float>(luaL_checknumber(L, 9)),
//        static_cast<float>(luaL_checknumber(L, 10)),
//        static_cast<float>(luaL_checknumber(L, 11)),
//        static_cast<float>(luaL_checknumber(L, 12)),
//        static_cast<float>(luaL_checknumber(L, 13))
//    ))
//    {
//        return luaL_error(L, "can't render '%m'.", luaL_checkstring(L, 1));
//    }
//    return 0;
}

void RenderModule::RenderTexture(LuaStack& stack, const char* textureName, const char* blend, AbsIndex vertex1, AbsIndex vertex2,
    AbsIndex vertex3, AbsIndex vertex4)
{
    // TODO
//    const char* tex_name = luaL_checkstring(L, 1);
//    BlendMode blend = TranslateBlendMode(L, 2);
//    f2dGraphics2DVertex vertex[4];
//
//    for (int i = 0; i < 4; ++i)
//    {
//        lua_pushinteger(L, 1);
//        lua_gettable(L, 3 + i);
//        vertex[i].x = (float)lua_tonumber(L, -1);
//
//        lua_pushinteger(L, 2);
//        lua_gettable(L, 3 + i);
//        vertex[i].y = (float)lua_tonumber(L, -1);
//
//        lua_pushinteger(L, 3);
//        lua_gettable(L, 3 + i);
//        vertex[i].z = (float)lua_tonumber(L, -1);
//
//        lua_pushinteger(L, 4);
//        lua_gettable(L, 3 + i);
//        vertex[i].u = (float)lua_tonumber(L, -1);
//
//        lua_pushinteger(L, 5);
//        lua_gettable(L, 3 + i);
//        vertex[i].v = (float)lua_tonumber(L, -1);
//
//        lua_pushinteger(L, 6);
//        lua_gettable(L, 3 + i);
//        vertex[i].color = static_cast<fcyColor*>(luaL_checkudata(L, -1, TYPENAME_COLOR))->argb;
//
//        lua_pop(L, 6);
//    }
//
//    if (!LAPP.RenderTexture(tex_name, blend, vertex))
//        return luaL_error(L, "can't render texture '%s'.", tex_name);
//    return 0;
}

void RenderModule::RenderText(const char* name, const char* text, double x, double y, std::optional<double> scale /* =1 */,
    std::optional<TextAlignment> align /* =5 */)
{
    // TODO
//    ResFont::FontAlignHorizontal halign = ResFont::FontAlignHorizontal::Center;
//    ResFont::FontAlignVertical valign = ResFont::FontAlignVertical::Middle;
//    if (lua_gettop(L) == 6)
//        TranslateAlignMode(L, 6, halign, valign);
//    if (!LAPP.RenderText(
//        luaL_checkstring(L, 1),
//        luaL_checkstring(L, 2),
//        (float)luaL_checknumber(L, 3),
//        (float)luaL_checknumber(L, 4),
//        (float)(luaL_optnumber(L, 5, 1.0) * LRES.GetGlobalImageScaleFactor()),
//        halign,
//        valign
//    ))
//    {
//        return luaL_error(L, "can't draw text '%m'.", luaL_checkstring(L, 1));
//    }
//    return 0;
}

void RenderModule::RenderTrueTypeFont(const char* name, const char* text, double left, double right, double bottom, double top, int32_t fmt,
    LSTGColor* blend)
{
    // TODO
//    if (!LAPP.RenderTTF(
//        luaL_checkstring(L, 1),
//        luaL_checkstring(L, 2),
//        (float)luaL_checknumber(L, 3),
//        (float)luaL_checknumber(L, 4),
//        (float)luaL_checknumber(L, 5),
//        (float)luaL_checknumber(L, 6),
//        LRES.GetGlobalImageScaleFactor() * (float)luaL_optnumber(L, 9, 1.0),
//        luaL_checkinteger(L, 7),
//        *static_cast<fcyColor*>(luaL_checkudata(L, 8, TYPENAME_COLOR))
//    ))
//    {
//        return luaL_error(L, "can't render font '%s'.", luaL_checkstring(L, 1));
//    }
//    return 0;
}

void RenderModule::PushRenderTarget(const char* name)
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

void RenderModule::PopRenderTarget()
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

void RenderModule::PostEffectCapture()
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
