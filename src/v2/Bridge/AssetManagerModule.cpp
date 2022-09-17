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
#include <lstg/v2/Asset/TrueTypeFontAsset.hpp>
#include <lstg/v2/Asset/HgeFontAsset.hpp>
#include <lstg/v2/Asset/EffectAsset.hpp>
#include <lstg/v2/Asset/HgeParticleAsset.hpp>
#include <lstg/v2/Asset/SoundAsset.hpp>
#include <lstg/v2/Asset/MusicAsset.hpp>
#include "detail/Helper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;
using namespace lstg::Subsystem;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(AssetManagerModule);

#define GET_CURRENT_POOL \
    auto assetPools = detail::GetGlobalApp().GetAssetPools(); \
    auto [_, currentAssetPool] = assetPools->GetCurrentAssetPool(); \
    if (!currentAssetPool) \
        stack.Error("can't load resource at this time."); \
    assert(currentAssetPool)

void AssetManagerModule::SetResourceStatus(LuaStack& stack, const char* pool)
{
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    if (::strcmp(pool, "global") == 0)
        assetPools->SetCurrentAssetPool(AssetPoolTypes::Global);
    else if (::strcmp(pool, "stage") == 0)
        assetPools->SetCurrentAssetPool(AssetPoolTypes::Stage);
    else if (::strcmp(pool, "none") == 0)
        assetPools->SetCurrentAssetPool(AssetPoolTypes::None);
    else
        stack.Error("invalid argument #1 for 'SetResourceStatus', requires 'stage', 'global' or 'none'.");
}

void AssetManagerModule::RemoveResource(LuaStack& stack, const char* pool, std::optional<AssetTypes> type, std::optional<const char*> name)
{
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    auto p = AssetPoolTypes::None;
    if (::strcmp(pool, "global") == 0)
        p = AssetPoolTypes::Global;
    else if (::strcmp(pool, "stage") == 0)
        p = AssetPoolTypes::Stage;
    else if (::strcmp(pool, "none") == 0)
        p = AssetPoolTypes::None;
    else
        stack.Error("invalid argument #1 for 'RemoveResource', requires 'stage', 'global' or 'none'.");

    if (type)
    {
        if (!name)
            stack.Error("argument #3 must be specified");
        assetPools->RemoveAsset(p, *type, *name);
    }
    else
    {
        assetPools->ClearAssetPool(p);
        if (p == AssetPoolTypes::Global)
            LSTG_LOG_INFO_CAT(AssetManagerModule, "Global asset pool is clear");
        else if (p == AssetPoolTypes::Stage)
            LSTG_LOG_INFO_CAT(AssetManagerModule, "Stage asset pool is clear");
    }
}

std::optional<const char*> AssetManagerModule::CheckRes(AssetTypes type, const char* name)
{
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // NOTE: 老版本行为会先去 GlobalPool 查找，再去 StagePool 查找
    // 这里统一先搜 StagePool
    auto p = assetPools->LocateAsset(type, name);
    switch (std::get<0>(p))
    {
        case AssetPoolTypes::Global:
            return "global";
        case AssetPoolTypes::Stage:
            return "stage";
        default:
            return nullopt;
    }
}

AssetManagerModule::Unpack<AssetManagerModule::AbsIndex, AssetManagerModule::AbsIndex> AssetManagerModule::EnumRes(LuaStack& stack,
    AssetTypes type)
{
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    AssetPool* pools[] = {
        assetPools->GetGlobalAssetPool().get(),
        assetPools->GetStageAssetPool().get()
    };

    auto base = stack.GetTop();
    for (auto assetPool : pools)
    {
        size_t idx = 1;
        lua_newtable(stack);
        assetPool->Visit([&](const AssetPtr& asset) -> std::tuple<bool, monostate> {
            if (IsAssetNameMatchType(asset->GetName(), type))
                stack.RawSet(-1, static_cast<int>(idx++), ExtractAssetName(asset->GetName()).c_str());
            return { false, {} };
        });
    }
    assert(stack.GetTop() == base + 2);
    assert(stack.TypeOf(base + 1) == LUA_TTABLE);
    assert(stack.TypeOf(base + 2) == LUA_TTABLE);
    return { AbsIndex(base + 1u), AbsIndex(base + 2u) };
}

AssetManagerModule::Unpack<double, double> AssetManagerModule::GetTextureSize(LuaStack& stack, const char* name)
{
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    auto asset = assetPools->FindAsset(AssetTypes::Texture, name);
    if (!asset)
        stack.Error("texture '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::TextureAsset::GetAssetTypeIdStatic());

    auto texAsset = static_pointer_cast<Asset::TextureAsset>(asset);
    return {texAsset->GetWidth(), texAsset->GetHeight()};
}

void AssetManagerModule::LoadTexture(LuaStack& stack, const char* name, const char* path, std::optional<bool> mipmap /* = false */)
{
    GET_CURRENT_POOL;

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
        {"rt", false},
        {"path", path},
        {"mipmaps", mipmap ? *mipmap : false},
    });
    if (!ret)
        stack.Error("load texture from file '%s' fail: %s", path, ret.GetError().message().c_str());
}

void AssetManagerModule::LoadImage(LuaStack& stack, const char* name, const char* textureName, double x, double y, double w, double h,
    std::optional<double> a, std::optional<double> b, std::optional<bool> rect)
{
    GET_CURRENT_POOL;

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
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    auto asset = assetPools->FindAsset(AssetTypes::Image, name);
    if (!asset)
        stack.Error("image '%s' not found.", name);
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
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    auto asset = assetPools->FindAsset(AssetTypes::Image, name);
    if (!asset)
        stack.Error("image '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SpriteAsset::GetAssetTypeIdStatic());

    auto spriteAsset = static_pointer_cast<Asset::SpriteAsset>(asset);
    spriteAsset->SetAnchor({x, y});
}

void AssetManagerModule::SetImageScale(double factor)
{
    LSTG_LOG_DEPRECATED(AssetManagerModule, SetImageScale);
}

void AssetManagerModule::LoadAnimation(LuaStack& stack, const char* name, const char* textureName, double x, double y, double w, double h,
    int32_t n, int32_t m, int32_t interval, std::optional<double> a, std::optional<double> b, std::optional<bool> rect)
{
    GET_CURRENT_POOL;

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
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    auto asset = assetPools->FindAsset(AssetTypes::Animation, name);
    if (!asset)
        stack.Error("animation '%s' not found.", name);
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
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    auto asset = assetPools->FindAsset(AssetTypes::Animation, name);
    if (!asset)
        stack.Error("animation '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SpriteSequenceAsset::GetAssetTypeIdStatic());

    auto spriteSequenceAsset = static_pointer_cast<Asset::SpriteSequenceAsset>(asset);
    spriteSequenceAsset->SetAnchor({x, y});
}

void AssetManagerModule::LoadParticle(LuaStack& stack, const char* name, const char* path, const char* imgName, std::optional<double> a,
    std::optional<double> b, std::optional<bool> rect)
{
    GET_CURRENT_POOL;

    auto assetSystem = detail::GetGlobalApp().GetSubsystem<AssetSystem>();
    assert(assetSystem);

    // 先检查资源是否存在，对于已经存在的资源，会跳过加载过程
    auto fullName = MakeFullAssetName(AssetTypes::Particle, name);
    if (currentAssetPool->ContainsAsset(fullName))
    {
        LSTG_LOG_WARN_CAT(AssetManagerModule, "Particle \"{}\" is already loaded", name);
        return;
    }

    // 构造参数
    nlohmann::json args {
        {"path", path},
        {"sprite", MakeFullAssetName(AssetTypes::Image, imgName)},
        {"colliderHalfSizeX", a ? *a : 0.},
        {"colliderHalfSizeY", b ? *b : (a ? *a : 0.)},
        {"colliderIsRect", rect && *rect},
    };

    // 执行加载
    auto ret = assetSystem->CreateAsset<Asset::HgeParticleAsset>(currentAssetPool, fullName, args);
    if (!ret)
        stack.Error("load particle \"%s\" fail: %s", name, ret.GetError().message().c_str());
}

void AssetManagerModule::LoadTexturedFont(LuaStack& stack, const char* name, const char* path, std::optional<bool> mipmap /* =true */)
{
    GET_CURRENT_POOL;

    auto assetSystem = detail::GetGlobalApp().GetSubsystem<AssetSystem>();
    assert(assetSystem);

    // 先检查资源是否存在，对于已经存在的资源，会跳过加载过程
    auto fullName = MakeFullAssetName(AssetTypes::TexturedFont, name);
    if (currentAssetPool->ContainsAsset(fullName))
    {
        LSTG_LOG_WARN_CAT(AssetManagerModule, "Textured font \"{}\" is already loaded", name);
        return;
    }

    // 构造参数
    nlohmann::json args {
        {"path", path},
        {"mipmaps", mipmap ? *mipmap : true},
    };

    // 执行加载
    auto ret = assetSystem->CreateAsset<Asset::HgeFontAsset>(currentAssetPool, fullName, args);
    if (!ret)
        stack.Error("load textured font \"%s\" fail: %s", name, ret.GetError().message().c_str());
}

void AssetManagerModule::SetTexturedFontState(LuaStack& stack, const char* name, const char* blend, std::optional<LSTGColor*> color)
{
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    auto asset = assetPools->FindAsset(AssetTypes::TexturedFont, name);
    if (!asset)
        stack.Error("textured font '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::HgeFontAsset::GetAssetTypeIdStatic());

    auto hgeFontAsset = static_pointer_cast<Asset::HgeFontAsset>(asset);

    // 转换混合模式
    BlendMode m(blend);
    hgeFontAsset->SetDefaultBlendMode(m);

    // 决定顶点色
    if (color)
        hgeFontAsset->SetDefaultBlendColor(**color);
}

void AssetManagerModule::SetTexturedFontState2()
{
    LSTG_LOG_DEPRECATED(AssetManagerModule, SetFontState2);
}

void AssetManagerModule::LoadTrueTypeFont(LuaStack& stack, const char* name, const char* path, double width, std::optional<double> height)
{
    GET_CURRENT_POOL;

    auto assetSystem = detail::GetGlobalApp().GetSubsystem<AssetSystem>();
    assert(assetSystem);

    // 先检查资源是否存在，对于已经存在的资源，会跳过加载过程
    auto fullName = MakeFullAssetName(AssetTypes::TrueTypeFont, name);
    if (currentAssetPool->ContainsAsset(fullName))
    {
        LSTG_LOG_WARN_CAT(AssetManagerModule, "TrueType font \"{}\" is already loaded", name);
        return;
    }

    // 构造参数
    auto fontSize = static_cast<uint32_t>(width);
    if (height)
    {
        if (width == 0)
            fontSize = static_cast<uint32_t>(*height);
        else if (static_cast<uint32_t>(*height) != width)
            LSTG_LOG_WARN_CAT(AssetManagerModule, "Create ttf font with height is deprecated", name);
    }
    fontSize = clamp(fontSize, 1u, 100u);  // 限定大小

    nlohmann::json args {
        {"path", path},
        {"size", fontSize},
    };

    // 执行加载
    auto ret = assetSystem->CreateAsset<Asset::TrueTypeFontAsset>(currentAssetPool, fullName, args);
    if (!ret)
        stack.Error("load ttf font \"%s\" fail: %s", name, ret.GetError().message().c_str());
}

void AssetManagerModule::RegTTF()
{
    LSTG_LOG_DEPRECATED(AssetManagerModule, RegTTF);
}

void AssetManagerModule::LoadSound(LuaStack& stack, const char* name, const char* path)
{
    GET_CURRENT_POOL;

    auto assetSystem = detail::GetGlobalApp().GetSubsystem<AssetSystem>();
    assert(assetSystem);

    // 先检查资源是否存在，对于已经存在的资源，会跳过加载过程
    auto fullName = MakeFullAssetName(AssetTypes::Sound, name);
    if (currentAssetPool->ContainsAsset(fullName))
    {
        LSTG_LOG_WARN_CAT(AssetManagerModule, "Sound \"{}\" is already loaded", name);
        return;
    }

    // 构造参数
    nlohmann::json args {
        {"path", path},
    };

    // 执行加载
    auto ret = assetSystem->CreateAsset<Asset::SoundAsset>(currentAssetPool, fullName, args);
    if (!ret)
        stack.Error("load sound \"%s\" from \"%s\" fail: %s", name, path, ret.GetError().message().c_str());
}

void AssetManagerModule::LoadMusic(LuaStack& stack, const char* name, const char* path, double end, double loop)
{
    GET_CURRENT_POOL;

    auto assetSystem = detail::GetGlobalApp().GetSubsystem<AssetSystem>();
    assert(assetSystem);

    // 先检查资源是否存在，对于已经存在的资源，会跳过加载过程
    auto fullName = MakeFullAssetName(AssetTypes::Music, name);
    if (currentAssetPool->ContainsAsset(fullName))
    {
        LSTG_LOG_WARN_CAT(AssetManagerModule, "Music \"{}\" is already loaded", name);
        return;
    }

    // 构造参数
    double loopStart = std::max(0., end - loop);
    nlohmann::json args {
        {"path", path},
        {"loopBeginMs", static_cast<int32_t>(loopStart * 1000)},
        {"loopEndMs", static_cast<int32_t>(end * 1000)},
    };

    // 执行加载
    auto ret = assetSystem->CreateAsset<Asset::MusicAsset>(currentAssetPool, fullName, args);
    if (!ret)
        stack.Error("load music \"%s\" from \"%s\" fail: %s", name, path, ret.GetError().message().c_str());
}

void AssetManagerModule::LoadFX(LuaStack& stack, const char* name, const char* path)
{
    GET_CURRENT_POOL;

    auto assetSystem = detail::GetGlobalApp().GetSubsystem<AssetSystem>();
    assert(assetSystem);

    // 先检查资源是否存在，对于已经存在的资源，会跳过加载过程
    auto fullName = MakeFullAssetName(AssetTypes::Effect, name);
    if (currentAssetPool->ContainsAsset(fullName))
    {
        LSTG_LOG_WARN_CAT(AssetManagerModule, "Effect \"{}\" is already loaded", name);
        return;
    }

    // 构造参数
    nlohmann::json args {
        {"path", path},
    };

    // 执行加载
    auto ret = assetSystem->CreateAsset<Asset::EffectAsset>(currentAssetPool, fullName, args);
    if (!ret)
        stack.Error("load fx \"%s\" fail: %s", name, ret.GetError().message().c_str());
}

void AssetManagerModule::CreateRenderTarget(LuaStack& stack, const char* name)
{
    GET_CURRENT_POOL;

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
        {"rt", true},
    });
    if (!ret)
        stack.Error("create render target fail: %s", ret.GetError().message().c_str());
}

bool AssetManagerModule::IsRenderTarget(LuaStack& stack, const char* name)
{
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    auto asset = assetPools->FindAsset(AssetTypes::Texture, name);
    if (!asset)
        stack.Error("render target '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::TextureAsset::GetAssetTypeIdStatic());

    auto tex = static_pointer_cast<Asset::TextureAsset>(asset);
    return tex->IsRenderTarget();
}
