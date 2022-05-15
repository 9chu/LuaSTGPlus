/**
 * @file
 * @author 9chu
 * @date 2022/4/19
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/AssetManagerModule.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/Asset/TextureAsset.hpp>
#include "detail/Global.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;
using namespace lstg::Subsystem;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(AssetManagerModule);

static const size_t kAssetPrefixLength = 4;

static const char* kAssetPrefix[] = {
    "???$",
    "tex$",  // Texture = 1
    "img$",  // Image = 2
    "ani$",  // Animation = 3
    "bgm$",  // Music = 4
    "snd$",  // Sound = 5
    "par$",  // Particle = 6
    "txf$",  // TexturedFont = 7
    "ttf$",  // TrueTypeFont = 8,
    "lfx$",  // Effect = 9,
};

namespace
{
    /**
     * 转换资产类型到前缀
     * @param t 资产类型
     * @return 前缀
     */
    const char* AssetTypeToPrefix(AssetManagerModule::AssetTypes t) noexcept
    {
        auto idx = static_cast<uint32_t>(t);
        if (idx >= std::extent_v<decltype(kAssetPrefix)>)
            idx = 0;
        return kAssetPrefix[idx];
    }

    /**
     * 检查资产名是否与类型匹配
     * @param name 名称
     * @param t 类型
     * @return 是否匹配
     */
    bool IsAssetNameMatchType(string_view name, AssetManagerModule::AssetTypes t) noexcept
    {
        auto prefix = AssetTypeToPrefix(t);
        if (name.length() >= kAssetPrefixLength && name.substr(0, kAssetPrefixLength) == prefix)
            return true;
        return false;
    }

    /**
     * 构造完整的资产名
     * @param t 资产类型
     * @param name 名称
     * @return 完整资产名
     */
    string MakeFullAssetName(AssetManagerModule::AssetTypes t, string_view name)
    {
        auto prefix = AssetTypeToPrefix(t);
        return fmt::format("{}{}", prefix, name);
    }

    /**
     * 解出资产名
     * @param name 完整资产名
     * @return 脚本系统中所用资产名
     */
    string ExtractAssetName(string_view name)
    {
        assert(name.length() >= kAssetPrefixLength);
        return string { name.substr(kAssetPrefixLength) };
    }
}

void AssetManagerModule::SetResourceStatus(LuaStack& stack, const char* pool)
{
    if (::strcmp(pool, "global") == 0)
        detail::GetGlobalApp().SetCurrentAssetPool(detail::GetGlobalApp().GetGlobalAssetPool());
    else if (::strcmp(pool, "stage") == 0)
        detail::GetGlobalApp().SetCurrentAssetPool(detail::GetGlobalApp().GetStageAssetPool());
    else if (::strcmp(pool, "none") == 0)
        detail::GetGlobalApp().SetCurrentAssetPool(nullptr);
    else
        stack.Error("invalid argument #1 for 'SetResourceStatus', requires 'stage', 'global' or 'none'.");
}

void AssetManagerModule::RemoveResource(LuaStack& stack, const char* pool, std::optional<AssetTypes> type, std::optional<const char*> name)
{
    AssetPool* assetPool = nullptr;
    if (::strcmp(pool, "global") == 0)
        assetPool = detail::GetGlobalApp().GetGlobalAssetPool().get();
    else if (::strcmp(pool, "stage") == 0)
        assetPool = detail::GetGlobalApp().GetStageAssetPool().get();
    else if (::strcmp(pool, "none") == 0)
        return;  // do nothing
    else
        stack.Error("invalid argument #1 for 'RemoveResource', requires 'stage', 'global' or 'none'.");

    assert(assetPool);
    if (type)
    {
        if (!name)
            stack.Error("argument #3 must be specified");

        auto fullName = MakeFullAssetName(*type, *name);
        auto ret = assetPool->RemoveAsset(fullName);
        if (!ret)
            LSTG_LOG_WARN_CAT(AssetManagerModule, "Remove asset {} fail: {}", fullName, ret.GetError());
    }
    else
    {
        assetPool->Clear(AssetPool::AssetClearTypes::All);
        if (assetPool == detail::GetGlobalApp().GetGlobalAssetPool().get())
        {
            LSTG_LOG_INFO_CAT(AssetManagerModule, "Global asset pool is clear");
        }
        else
        {
            assert(assetPool == detail::GetGlobalApp().GetStageAssetPool().get());
            LSTG_LOG_INFO_CAT(AssetManagerModule, "Stage asset pool is clear");
        }
    }
}

std::optional<const char*> AssetManagerModule::CheckRes(AssetTypes type, const char* name)
{
    auto fullName = MakeFullAssetName(type, name);

    // 先在全局池中寻找再到关卡池中找
    // FIXME: 行为似乎不统一，这个应该改成先在关卡池找再去全局池找？
    if (detail::GetGlobalApp().GetGlobalAssetPool()->ContainsAsset(fullName))
        return "global";
    else if (detail::GetGlobalApp().GetStageAssetPool()->ContainsAsset(fullName))
        return "stage";
    return nullopt;
}

AssetManagerModule::Unpack<AssetManagerModule::AbsIndex, AssetManagerModule::AbsIndex> AssetManagerModule::EnumRes(LuaStack& stack,
    AssetTypes type)
{
    AssetPool* assetPools[] = {
        detail::GetGlobalApp().GetGlobalAssetPool().get(),
        detail::GetGlobalApp().GetStageAssetPool().get()
    };

    for (auto assetPool : assetPools)
    {
        size_t idx = 1;
        lua_newtable(stack);
        assetPool->Visit([&](const AssetPtr& asset) -> std::tuple<bool, monostate> {
            if (IsAssetNameMatchType(asset->GetName(), type))
                lua_setfield(stack, static_cast<int>(idx++), ExtractAssetName(asset->GetName()).c_str());
            return { false, {} };
        });
    }
    assert(stack.GetTop() == 2);
    assert(stack.TypeOf(1) == LUA_TTABLE);
    assert(stack.TypeOf(2) == LUA_TTABLE);
    return { AbsIndex(1u), AbsIndex(2u) };
}

AssetManagerModule::Unpack<double, double> AssetManagerModule::GetTextureSize(LuaStack& stack, const char* name)
{
    auto fullName = MakeFullAssetName(AssetTypes::Texture, name);
    auto asset = detail::GetGlobalApp().FindAsset(fullName);
    if (!asset)
        stack.Error("texture '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == TextureAsset::GetAssetTypeIdStatic());

    auto texAsset = static_pointer_cast<TextureAsset>(asset);
    return {texAsset->GetWidth(), texAsset->GetHeight()};
}

void AssetManagerModule::LoadTexture(LuaStack& stack, const char* name, const char* path, std::optional<bool> mipmap /* = false */)
{
    const auto& currentAssetPool = detail::GetGlobalApp().GetCurrentAssetPool();
    if (!currentAssetPool)
        stack.Error("can't load resource at this time.");
    assert(currentAssetPool);

    auto assetSystem = detail::GetGlobalApp().GetSubsystem<AssetSystem>();
    assert(assetSystem);

    // 先检查资源是否存在，对于已经存在的资源，会跳过加载过程
    auto fullName = MakeFullAssetName(AssetTypes::Texture, name);
    if (currentAssetPool->ContainsAsset(fullName))
    {
        LSTG_LOG_WARN_CAT(AssetManagerModule, "Texture \"{}\" is already loaded", name);
        return;
    }

    // 执行加载
    auto ret = assetSystem->CreateAsset<TextureAsset>(currentAssetPool, fullName, nlohmann::json {
        {"path", path},
        {"mipmaps", mipmap ? *mipmap : false},
    });
    if (!ret)
        stack.Error("can't load texture from file '%s'.", path);
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
