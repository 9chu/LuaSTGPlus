/**
 * @file
 * @author 9chu
 * @date 2022/4/19
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/AssetManagerModule.hpp>

#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

LSTG_DEF_LOG_CATEGORY(AssetManagerModule);

void AssetManagerModule::SetResourceStatus(const char* pool)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    if (strcmp(s, "global") == 0)
//        LRES.SetActivedPoolType(ResourcePoolType::Global);
//    else if (strcmp(s, "stage") == 0)
//        LRES.SetActivedPoolType(ResourcePoolType::Stage);
//    else if (strcmp(s, "none") == 0)
//        LRES.SetActivedPoolType(ResourcePoolType::None);
//    else
//        return luaL_error(L, "invalid argument #1 for 'SetResourceStatus', requires 'stage', 'global' or 'none'.");
//    return 0;
}

void AssetManagerModule::RemoveResource(const char* pool, std::optional<AssetTypes> type, std::optional<const char*> name)
{
    // TODO
//    ResourcePoolType t;
//    const char* s = luaL_checkstring(L, 1);
//    if (strcmp(s, "global") == 0)
//        t = ResourcePoolType::Global;
//    else if (strcmp(s, "stage") == 0)
//        t = ResourcePoolType::Stage;
//    else if (strcmp(s, "none") != 0)
//        t = ResourcePoolType::None;
//    else
//        return luaL_error(L, "invalid argument #1 for 'RemoveResource', requires 'stage', 'global' or 'none'.");
//
//    if (lua_gettop(L) == 1)
//    {
//        switch (t)
//        {
//            case ResourcePoolType::Stage:
//                LRES.GetResourcePool(ResourcePoolType::Stage)->Clear();
//                LINFO("关卡资源池已清空");
//                break;
//            case ResourcePoolType::Global:
//                LRES.GetResourcePool(ResourcePoolType::Global)->Clear();
//                LINFO("全局资源池已清空");
//                break;
//            default:
//                break;
//        }
//    }
//    else
//    {
//        ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 2));
//        const char* tResourceName = luaL_checkstring(L, 3);
//
//        switch (t)
//        {
//            case ResourcePoolType::Stage:
//                LRES.GetResourcePool(ResourcePoolType::Stage)->RemoveResource(tResourceType, tResourceName);
//                break;
//            case ResourcePoolType::Global:
//                LRES.GetResourcePool(ResourcePoolType::Global)->RemoveResource(tResourceType, tResourceName);
//                break;
//            default:
//                break;
//        }
//    }
//
//    return 0;
}

std::optional<const char*> AssetManagerModule::CheckRes(AssetTypes type, const char* name)
{
    // TODO
//    ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 1));
//    const char* tResourceName = luaL_checkstring(L, 2);
//    // 先在全局池中寻找再到关卡池中找
//    if (LRES.GetResourcePool(ResourcePoolType::Global)->CheckResourceExists(tResourceType, tResourceName))
//        lua_pushstring(L, "global");
//    else if (LRES.GetResourcePool(ResourcePoolType::Stage)->CheckResourceExists(tResourceType, tResourceName))
//        lua_pushstring(L, "stage");
//    else
//        lua_pushnil(L);
//    return 1;
    return nullptr_t {};
}

AssetManagerModule::Unpack<AssetManagerModule::AbsIndex, AssetManagerModule::AbsIndex> AssetManagerModule::EnumRes(AssetTypes type)
{
    // TODO
//    ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 1));
//    LRES.GetResourcePool(ResourcePoolType::Global)->ExportResourceList(L, tResourceType);
//    LRES.GetResourcePool(ResourcePoolType::Stage)->ExportResourceList(L, tResourceType);
//    return 2;
    return { {}, {} };
}

AssetManagerModule::Unpack<double, double> AssetManagerModule::GetTextureSize(const char* name)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//    fcyVec2 size;
//    if (!LRES.GetTextureSize(name, size))
//        return luaL_error(L, "texture '%s' not found.", name);
//    lua_pushnumber(L, size.x);
//    lua_pushnumber(L, size.y);
//    return 2;
    return {0, 0};
}

void AssetManagerModule::LoadTexture(const char* name, const char* path, std::optional<bool> mipmap /* = false */)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//    const char* path = luaL_checkstring(L, 2);
//
//    ResourcePool* pActivedPool = LRES.GetActivedPool();
//    if (!pActivedPool)
//        return luaL_error(L, "can't load resource at this time.");
//    if (!pActivedPool->LoadTexture(name, path, lua_toboolean(L, 3) == 0 ? false : true))
//        return luaL_error(L, "can't load texture from file '%s'.", path);
//    return 0;
}

void AssetManagerModule::LoadImage(const char* name, const char* textureName, double x, double y, double w, double h,
    std::optional<double> a, std::optional<double> b, std::optional<bool> rect)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//    const char* texname = luaL_checkstring(L, 2);
//
//    ResourcePool* pActivedPool = LRES.GetActivedPool();
//    if (!pActivedPool)
//        return luaL_error(L, "can't load resource at this time.");
//
//    if (!pActivedPool->LoadImage(
//        name,
//        texname,
//        luaL_checknumber(L, 3),
//        luaL_checknumber(L, 4),
//        luaL_checknumber(L, 5),
//        luaL_checknumber(L, 6),
//        luaL_optnumber(L, 7, 0.),
//        luaL_optnumber(L, 8, 0.),
//        lua_toboolean(L, 9) == 0 ? false : true
//    ))
//    {
//        return luaL_error(L, "load image failed (name='%s', tex='%s').", name, texname);
//    }
//    return 0;
}

void AssetManagerModule::SetImageState(const char* name, const char* blend, std::optional<LSTGColor*> vertexColor1,
    std::optional<LSTGColor*> vertexColor2, std::optional<LSTGColor*> vertexColor3, std::optional<LSTGColor*> vertexColor4)
{
    // TODO
//    ResSprite* p = LRES.FindSprite(luaL_checkstring(L, 1));
//    if (!p)
//        return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
//
//    p->SetBlendMode(TranslateBlendMode(L, 2));
//    if (lua_gettop(L) == 3)
//        p->GetSprite()->SetColor(*static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR)));
//    else if (lua_gettop(L) == 6)
//    {
//        fcyColor tColors[] = {
//            *static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR)),
//            *static_cast<fcyColor*>(luaL_checkudata(L, 4, TYPENAME_COLOR)),
//            *static_cast<fcyColor*>(luaL_checkudata(L, 5, TYPENAME_COLOR)),
//            *static_cast<fcyColor*>(luaL_checkudata(L, 6, TYPENAME_COLOR))
//        };
//        p->GetSprite()->SetColor(tColors);
//    }
//    return 0;
}

void AssetManagerModule::SetImageCenter(const char* name, double x, double y)
{
    // TODO
//    ResSprite* p = LRES.FindSprite(luaL_checkstring(L, 1));
//    if (!p)
//        return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
//    p->GetSprite()->SetHotSpot(fcyVec2(
//        static_cast<float>(luaL_checknumber(L, 2) + p->GetSprite()->GetTexRect().a.x),
//        static_cast<float>(luaL_checknumber(L, 3) + p->GetSprite()->GetTexRect().a.y)));
//    return 0;
}

void AssetManagerModule::SetImageScale(double factor)
{
    // TODO
//    float x = static_cast<float>(luaL_checknumber(L, 1));
//    if (x == 0.f)
//        return luaL_error(L, "invalid argument #1 for 'SetImageScale'.");
//    LRES.SetGlobalImageScaleFactor(x);
//    return 0;
}

void AssetManagerModule::LoadAnimation(const char* name, const char* textureName, double x, double y, double w, double h, int32_t n,
    int32_t m, int32_t interval, std::optional<double> a, std::optional<double> b, std::optional<bool> rect)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//    const char* texname = luaL_checkstring(L, 2);
//
//    ResourcePool* pActivedPool = LRES.GetActivedPool();
//    if (!pActivedPool)
//        return luaL_error(L, "can't load resource at this time.");
//
//    if (!pActivedPool->LoadAnimation(
//        name,
//        texname,
//        luaL_checknumber(L, 3),
//        luaL_checknumber(L, 4),
//        luaL_checknumber(L, 5),
//        luaL_checknumber(L, 6),
//        luaL_checkinteger(L, 7),
//        luaL_checkinteger(L, 8),
//        luaL_checkinteger(L, 9),
//        luaL_optnumber(L, 10, 0.0f),
//        luaL_optnumber(L, 11, 0.0f),
//        lua_toboolean(L, 12) == 0 ? false : true
//    ))
//    {
//        return luaL_error(L, "load animation failed (name='%s', tex='%s').", name, texname);
//    }
//
//    return 0;
}

void AssetManagerModule::SetAnimationState(const char* name, const char* blend, std::optional<LSTGColor*> vertexColor1,
    std::optional<LSTGColor*> vertexColor2, std::optional<LSTGColor*> vertexColor3, std::optional<LSTGColor*> vertexColor4)
{
    // TODO
//    ResAnimation* p = LRES.FindAnimation(luaL_checkstring(L, 1));
//    if (!p)
//        return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));
//
//    p->SetBlendMode(TranslateBlendMode(L, 2));
//    if (lua_gettop(L) == 3)
//    {
//        fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR));
//        for (size_t i = 0; i < p->GetCount(); ++i)
//            p->GetSprite(i)->SetColor(c);
//    }
//    else if (lua_gettop(L) == 6)
//    {
//        fcyColor tColors[] = {
//            *static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR)),
//            *static_cast<fcyColor*>(luaL_checkudata(L, 4, TYPENAME_COLOR)),
//            *static_cast<fcyColor*>(luaL_checkudata(L, 5, TYPENAME_COLOR)),
//            *static_cast<fcyColor*>(luaL_checkudata(L, 6, TYPENAME_COLOR))
//        };
//        for (size_t i = 0; i < p->GetCount(); ++i)
//            p->GetSprite(i)->SetColor(tColors);
//    }
//    return 0;
}

void AssetManagerModule::SetAnimationCenter(const char* name, double x, double y)
{
    // TODO
//    ResAnimation* p = LRES.FindAnimation(luaL_checkstring(L, 1));
//    if (!p)
//        return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));
//    for (size_t i = 0; i < p->GetCount(); ++i)
//    {
//        p->GetSprite(i)->SetHotSpot(fcyVec2(
//            static_cast<float>(luaL_checknumber(L, 2) + p->GetSprite(i)->GetTexRect().a.x),
//            static_cast<float>(luaL_checknumber(L, 3) + p->GetSprite(i)->GetTexRect().a.y)));
//    }
//    return 0;
}

void AssetManagerModule::LoadParticle(const char* name, const char* path, const char* imgName, std::optional<double> a,
    std::optional<double> b, std::optional<bool> rect)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//    const char* path = luaL_checkstring(L, 2);
//    const char* img_name = luaL_checkstring(L, 3);
//
//    ResourcePool* pActivedPool = LRES.GetActivedPool();
//    if (!pActivedPool)
//        return luaL_error(L, "can't load resource at this time.");
//
//    if (!pActivedPool->LoadParticle(
//        name,
//        path,
//        img_name,
//        luaL_optnumber(L, 4, 0.0f),
//        luaL_optnumber(L, 5, 0.0f),
//        lua_toboolean(L, 6) == 0 ? false : true
//    ))
//    {
//        return luaL_error(L, "load particle failed (name='%s', file='%s', img='%s').", name, path, img_name);
//    }
//
//    return 0;
}

void AssetManagerModule::LoadTexturedFont(const char* name, const char* path,
    std::optional<std::variant<const char*, bool>> textureNamePathOrMipmap, std::optional<bool> mipmap /* =true */)
{
    // TODO
//    bool bSucceed = false;
//    const char* name = luaL_checkstring(L, 1);
//    const char* path = luaL_checkstring(L, 2);
//
//    ResourcePool* pActivedPool = LRES.GetActivedPool();
//    if (!pActivedPool)
//        return luaL_error(L, "can't load resource at this time.");
//
//    if (lua_gettop(L) == 2)
//    {
//        // HGE字体 mipmap=true
//        bSucceed = pActivedPool->LoadSpriteFont(name, path);
//    }
//    else
//    {
//        if (lua_isboolean(L, 3))
//        {
//            // HGE字体 mipmap=user_defined
//            bSucceed = pActivedPool->LoadSpriteFont(name, path, lua_toboolean(L, 3) == 0 ? false : true);
//        }
//        else
//        {
//            // fancy2d字体
//            const char* texpath = luaL_checkstring(L, 3);
//            if (lua_gettop(L) == 4)
//                bSucceed = pActivedPool->LoadSpriteFont(name, path, texpath, lua_toboolean(L, 4) == 0 ? false : true);
//            else
//                bSucceed = pActivedPool->LoadSpriteFont(name, path, texpath);
//        }
//    }
//
//    if (!bSucceed)
//        return luaL_error(L, "can't load font from file '%s'.", path);
//    return 0;
}

void AssetManagerModule::SetTexturedFontState(const char* name, const char* blend, std::optional<LSTGColor*> color)
{
    // TODO
//    ResFont* p = LRES.FindSpriteFont(luaL_checkstring(L, 1));
//    if (!p)
//        return luaL_error(L, "sprite font '%s' not found.", luaL_checkstring(L, 1));
//
//    p->SetBlendMode(TranslateBlendMode(L, 2));
//    if (lua_gettop(L) == 3)
//    {
//        fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR));
//        p->SetBlendColor(c);
//    }
//    return 0;
}

void AssetManagerModule::SetTexturedFontState2()
{
    LSTG_LOG_WARN_CAT(AssetManagerModule, "SetFontState2 is deprecated and has no effect anymore");
}

void AssetManagerModule::LoadTrueTypeFont(const char* name, const char* path, double width, double height)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//    const char* path = luaL_checkstring(L, 2);
//
//    ResourcePool* pActivedPool = LRES.GetActivedPool();
//    if (!pActivedPool)
//        return luaL_error(L, "can't load resource at this time.");
//
//    if (!pActivedPool->LoadTTFFont(name, path, (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4)))
//        return luaL_error(L, "load ttf font failed (name=%s, path=%s)", name, path);
//    return 0;
}

void AssetManagerModule::RegTTF()
{
    LSTG_LOG_WARN_CAT(AssetManagerModule, "RegTTF is deprecated and has no effect anymore");
}

void AssetManagerModule::LoadSound(const char* name, const char* path)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//    const char* path = luaL_checkstring(L, 2);
//
//    ResourcePool* pActivedPool = LRES.GetActivedPool();
//    if (!pActivedPool)
//        return luaL_error(L, "can't load resource at this time.");
//
//    if (!pActivedPool->LoadSound(name, path))
//        return luaL_error(L, "load sound failed (name=%s, path=%s)", name, path);
//    return 0;
}

void AssetManagerModule::LoadMusic(const char* name, const char* path, double end, double loop)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//    const char* path = luaL_checkstring(L, 2);
//
//    ResourcePool* pActivedPool = LRES.GetActivedPool();
//    if (!pActivedPool)
//        return luaL_error(L, "can't load resource at this time.");
//
//    double loop_end = luaL_checknumber(L, 3);
//    double loop_duration = luaL_checknumber(L, 4);
//    double loop_start = max(0., loop_end - loop_duration);
//
//    if (!pActivedPool->LoadMusic(
//        name,
//        path,
//        loop_start,
//        loop_end
//    ))
//    {
//        return luaL_error(L, "load music failed (name=%s, path=%s, loop=%f~%f)", name, path, loop_start, loop_end);
//    }
//    return 0;
}

void AssetManagerModule::LoadFX(const char* name, const char* path)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//    const char* path = luaL_checkstring(L, 2);
//
//    ResourcePool* pActivedPool = LRES.GetActivedPool();
//    if (!pActivedPool)
//        return luaL_error(L, "can't load resource at this time.");
//
//    if (!pActivedPool->LoadFX(name, path))
//        return luaL_error(L, "load fx failed (name=%s, path=%s)", name, path);
//    return 0;
}

void AssetManagerModule::CreateRenderTarget(const char* name)
{
    // TODO
//    const char* name = luaL_checkstring(L, 1);
//
//    ResourcePool* pActivedPool = LRES.GetActivedPool();
//    if (!pActivedPool)
//        return luaL_error(L, "can't load resource at this time.");
//
//    if (!pActivedPool->CreateRenderTarget(name))
//        return luaL_error(L, "can't create render target with name '%s'.", name);
//    return 0;
}

void AssetManagerModule::IsRenderTarget(const char* name)
{
    // TODO
//    ResTexture* p = LRES.FindTexture(luaL_checkstring(L, 1));
//    if (!p)
//        return luaL_error(L, "render target '%s' not found.", luaL_checkstring(L, 1));
//    lua_pushboolean(L, p->IsRenderTarget());
//    return 1;
}
