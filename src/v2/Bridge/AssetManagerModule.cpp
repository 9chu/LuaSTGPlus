/**
 * @file
 * @author 9chu
 * @date 2022/4/19
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/AssetManagerModule.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/v2/Asset/TextureAsset.hpp>
#include <lstg/v2/Asset/SpriteAsset.hpp>
#include <lstg/v2/Asset/SpriteSequenceAsset.hpp>
#include "detail/Helper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;
using namespace lstg::Subsystem;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(AssetManagerModule);

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
    assert(asset->GetAssetTypeId() == Asset::TextureAsset::GetAssetTypeIdStatic());

    auto texAsset = static_pointer_cast<Asset::TextureAsset>(asset);
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
    auto ret = assetSystem->CreateAsset<Asset::TextureAsset>(currentAssetPool, fullName, nlohmann::json {
        {"path", path},
        {"mipmaps", mipmap ? *mipmap : false},
    });
    if (!ret)
        stack.Error("load texture from file '%s' fail: %s", path, ret.GetError().message().c_str());
}

void AssetManagerModule::LoadImage(LuaStack& stack, const char* name, const char* textureName, double x, double y, double w, double h,
    std::optional<double> a, std::optional<double> b, std::optional<bool> rect)
{
    const auto& currentAssetPool = detail::GetGlobalApp().GetCurrentAssetPool();
    if (!currentAssetPool)
        stack.Error("can't load resource at this time.");
    assert(currentAssetPool);

    auto assetSystem = detail::GetGlobalApp().GetSubsystem<AssetSystem>();
    assert(assetSystem);

    // 先检查资源是否存在，对于已经存在的资源，会跳过加载过程
    auto fullName = MakeFullAssetName(AssetTypes::Image, name);
    if (currentAssetPool->ContainsAsset(fullName))
    {
        LSTG_LOG_WARN_CAT(AssetManagerModule, "Sprite \"{}\" is already loaded", name);
        return;
    }

    // 构造参数
    nlohmann::json args {
        {"texture", MakeFullAssetName(AssetTypes::Texture, textureName)},
        {"left", x},
        {"top", y},
        {"width", w},
        {"height", h},
        {"colliderHalfSizeX", a ? *a : 0.},
        {"colliderHalfSizeY", b ? *b : (a ? *a : 0.)},
        {"colliderIsRect", rect && *rect},
    };

    // 执行加载
    auto ret = assetSystem->CreateAsset<Asset::SpriteAsset>(currentAssetPool, fullName, args);
    if (!ret)
        stack.Error("load image \"%s\" fail: %s", name, ret.GetError().message().c_str());
}

void AssetManagerModule::SetImageState(LuaStack& stack, const char* name, const char* blend, std::optional<LSTGColor*> vertexColor1,
    std::optional<LSTGColor*> vertexColor2, std::optional<LSTGColor*> vertexColor3, std::optional<LSTGColor*> vertexColor4)
{
    auto fullName = MakeFullAssetName(AssetTypes::Image, name);
    auto asset = detail::GetGlobalApp().FindAsset(fullName);
    if (!asset)
        stack.Error("image '%s' not found", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SpriteAsset::GetAssetTypeIdStatic());

    if (!((vertexColor1 && vertexColor2 && vertexColor3 && vertexColor4) ||
        (!vertexColor1 && !vertexColor2 && !vertexColor3 && !vertexColor4) ||
        (vertexColor1 && !vertexColor2 && !vertexColor3 && !vertexColor4)))
    {
        stack.Error("number of vertex color components must be 0, 1 or 4");
    }

    auto spriteAsset = static_pointer_cast<Asset::SpriteAsset>(asset);

    // 转换混合模式
    BlendMode m(blend);
    spriteAsset->SetDefaultBlendMode(m);

    // 决定顶点色
    array<Render::ColorRGBA32, 4> vertexColor;
    if (vertexColor1 && vertexColor2 && vertexColor3 && vertexColor4)
    {
        assert(*vertexColor1 && *vertexColor2 && *vertexColor3 && *vertexColor4);
        vertexColor[0] = *(*vertexColor1);
        vertexColor[1] = *(*vertexColor2);
        vertexColor[2] = *(*vertexColor3);
        vertexColor[3] = *(*vertexColor4);
        spriteAsset->SetDefaultBlendColor(vertexColor);
    }
    else if (vertexColor1)
    {
        assert(*vertexColor1);
        vertexColor[0] = *(*vertexColor1);
        vertexColor[1] = *(*vertexColor1);
        vertexColor[2] = *(*vertexColor1);
        vertexColor[3] = *(*vertexColor1);
        spriteAsset->SetDefaultBlendColor(vertexColor);
    }
}

void AssetManagerModule::SetImageCenter(LuaStack& stack, const char* name, double x, double y)
{
    auto fullName = MakeFullAssetName(AssetTypes::Image, name);
    auto asset = detail::GetGlobalApp().FindAsset(fullName);
    if (!asset)
        stack.Error("image '%s' not found", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SpriteAsset::GetAssetTypeIdStatic());

    auto spriteAsset = static_pointer_cast<Asset::SpriteAsset>(asset);
    spriteAsset->SetAnchor({x, y});
}

void AssetManagerModule::SetImageScale(double factor)
{
    LSTG_LOG_WARN_CAT(AssetManagerModule, "SetImageScale is deprecated and has no effect anymore");
}

void AssetManagerModule::LoadAnimation(LuaStack& stack, const char* name, const char* textureName, double x, double y, double w, double h,
    int32_t n, int32_t m, int32_t interval, std::optional<double> a, std::optional<double> b, std::optional<bool> rect)
{
    const auto& currentAssetPool = detail::GetGlobalApp().GetCurrentAssetPool();
    if (!currentAssetPool)
        stack.Error("can't load resource at this time.");
    assert(currentAssetPool);

    auto assetSystem = detail::GetGlobalApp().GetSubsystem<AssetSystem>();
    assert(assetSystem);

    // 先检查资源是否存在，对于已经存在的资源，会跳过加载过程
    auto fullName = MakeFullAssetName(AssetTypes::Animation, name);
    if (currentAssetPool->ContainsAsset(fullName))
    {
        LSTG_LOG_WARN_CAT(AssetManagerModule, "animation \"{}\" is already loaded", name);
        return;
    }

    // 构造参数
    nlohmann::json args {
        {"texture", MakeFullAssetName(AssetTypes::Texture, textureName)},
        {"left", x},
        {"top", y},
        {"frameWidth", w},
        {"frameHeight", h},
        {"row", m},
        {"column", n},
        {"interval", interval},
        {"colliderHalfSizeX", a ? *a : 0.},
        {"colliderHalfSizeY", b ? *b : (a ? *a : 0.)},
        {"colliderIsRect", rect && *rect},
    };

    // 执行加载
    auto ret = assetSystem->CreateAsset<Asset::SpriteSequenceAsset>(currentAssetPool, fullName, args);
    if (!ret)
        stack.Error("load animation \"%s\" fail: %s", name, ret.GetError().message().c_str());
}

void AssetManagerModule::SetAnimationState(LuaStack& stack, const char* name, const char* blend, std::optional<LSTGColor*> vertexColor1,
    std::optional<LSTGColor*> vertexColor2, std::optional<LSTGColor*> vertexColor3, std::optional<LSTGColor*> vertexColor4)
{
    auto fullName = MakeFullAssetName(AssetTypes::Animation, name);
    auto asset = detail::GetGlobalApp().FindAsset(fullName);
    if (!asset)
        stack.Error("animation '%s' not found", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SpriteSequenceAsset::GetAssetTypeIdStatic());

    if (!((vertexColor1 && vertexColor2 && vertexColor3 && vertexColor4) ||
        (!vertexColor1 && !vertexColor2 && !vertexColor3 && !vertexColor4) ||
        (vertexColor1 && !vertexColor2 && !vertexColor3 && !vertexColor4)))
    {
        stack.Error("number of vertex color components must be 0, 1 or 4");
    }

    auto spriteSequenceAsset = static_pointer_cast<Asset::SpriteSequenceAsset>(asset);

    // 转换混合模式
    BlendMode m(blend);
    spriteSequenceAsset->SetDefaultBlendMode(m);

    // 决定顶点色
    array<Render::ColorRGBA32, 4> vertexColor;
    if (vertexColor1 && vertexColor2 && vertexColor3 && vertexColor4)
    {
        assert(*vertexColor1 && *vertexColor2 && *vertexColor3 && *vertexColor4);
        vertexColor[0] = *(*vertexColor1);
        vertexColor[1] = *(*vertexColor2);
        vertexColor[2] = *(*vertexColor3);
        vertexColor[3] = *(*vertexColor4);
        spriteSequenceAsset->SetDefaultBlendColor(vertexColor);
    }
    else if (vertexColor1)
    {
        assert(*vertexColor1);
        vertexColor[0] = *(*vertexColor1);
        vertexColor[1] = *(*vertexColor1);
        vertexColor[2] = *(*vertexColor1);
        vertexColor[3] = *(*vertexColor1);
        spriteSequenceAsset->SetDefaultBlendColor(vertexColor);
    }
}

void AssetManagerModule::SetAnimationCenter(LuaStack& stack, const char* name, double x, double y)
{
    auto fullName = MakeFullAssetName(AssetTypes::Animation, name);
    auto asset = detail::GetGlobalApp().FindAsset(fullName);
    if (!asset)
        stack.Error("animation '%s' not found", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SpriteSequenceAsset::GetAssetTypeIdStatic());

    auto spriteSequenceAsset = static_pointer_cast<Asset::SpriteSequenceAsset>(asset);
    spriteSequenceAsset->SetAnchor({x, y});
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
