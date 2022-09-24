/**
 * @file
 * @date 2022/7/7
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/Drawing2D/ParticleConfig.hpp>

#include <string>
#include <lstg/Core/Text/JsonHelper.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Drawing2D;

using namespace lstg::Text;
using namespace lstg::Subsystem::VFS;

namespace
{
    template <typename T>
    Result<void> Read(RandomRange& range, IStream* stream, T tag) noexcept
    {
        Result<void> ret;
        if (!(ret = Read(std::get<0>(range), stream, tag)))
            return ret.GetError();
        if (!(ret = Read(std::get<1>(range), stream, tag)))
            return ret.GetError();
        return {};
    }

    template <typename T>
    Result<void> Read(glm::vec2& out, IStream* stream, T tag) noexcept
    {
        Result<void> ret;
        if (!(ret = Read(out.x, stream, tag)))
            return ret.GetError();
        if (!(ret = Read(out.y, stream, tag)))
            return ret.GetError();
        return {};
    }

    template <typename T>
    Result<void> Read(glm::vec4& out, IStream* stream, T tag) noexcept
    {
        Result<void> ret;
        if (!(ret = Read(out.x, stream, tag)))
            return ret.GetError();
        if (!(ret = Read(out.y, stream, tag)))
            return ret.GetError();
        if (!(ret = Read(out.z, stream, tag)))
            return ret.GetError();
        if (!(ret = Read(out.w, stream, tag)))
            return ret.GetError();
        return {};
    }

    template <typename T>
    bool Read(T& out, const nlohmann::json& json, std::string_view name) noexcept
    {
        char buf[128];
        ::memset(buf, 0, sizeof(buf));
        buf[0] = '/';
        ::strncpy(&buf[1], name.data(), std::min(name.size(), sizeof(buf) - 2));
        if (buf[1] >= 'A' && buf[1] <= 'Z')
            buf[1] = static_cast<char>(buf[1] - 'A' + 'a');

        auto ret = JsonHelper::ReadValue<T>(json, buf);
        if (!ret)
            return false;
        out = std::move(*ret);
        return {};
    }

    bool Read(RandomRange& range, const nlohmann::json& json, std::string_view name) noexcept
    {
        char buf[128];
        ::memset(buf, 0, sizeof(buf));
        buf[0] = '/';
        ::strncpy(&buf[1], name.data(), std::min(name.size(), sizeof(buf) - 2));
        if (buf[1] >= 'A' && buf[1] <= 'Z')
            buf[1] = static_cast<char>(buf[1] - 'A' + 'a');

        auto ret = JsonHelper::ReadValue<string>(json, buf);
        if (!ret)
            return false;
        ::sscanf(ret->c_str(), "%f,%f", &std::get<0>(range), &std::get<1>(range));
        return {};
    }

    bool Read(glm::vec2& out, const nlohmann::json& json, std::string_view name) noexcept
    {
        char buf[128];
        ::memset(buf, 0, sizeof(buf));
        buf[0] = '/';
        ::strncpy(&buf[1], name.data(), std::min(name.size(), sizeof(buf) - 2));
        if (buf[1] >= 'A' && buf[1] <= 'Z')
            buf[1] = static_cast<char>(buf[1] - 'A' + 'a');

        auto ret = JsonHelper::ReadValue<string>(json, buf);
        if (!ret)
            return false;
        float x = 0.f, y = 0.f;
        ::sscanf(ret->c_str(), "%f,%f", &x, &y);
        out = { x, y };
        return {};
    }

    bool Read(glm::vec4& out, const nlohmann::json& json, std::string_view name) noexcept
    {
        char buf[128];
        ::memset(buf, 0, sizeof(buf));
        buf[0] = '/';
        ::strncpy(&buf[1], name.data(), std::min(name.size(), sizeof(buf) - 2));
        if (buf[1] >= 'A' && buf[1] <= 'Z')
            buf[1] = static_cast<char>(buf[1] - 'A' + 'a');

        auto ret = JsonHelper::ReadValue<string>(json, buf);
        if (!ret)
            return false;
        float r = 0, g = 0, b = 0, a = 0;
        ::sscanf(ret->c_str(), "%f,%f,%f,%f", &r, &g, &b, &a);
        out = {
            clamp(r, 0.f, 1.f),
            clamp(g, 0.f, 1.f),
            clamp(b, 0.f, 1.f),
            clamp(a, 0.f, 1.f),
        };
        return {};
    }
}

#define READ_CONFIG(NAME) \
    do { \
        Result<void> ret = Read(NAME, stream, LittleEndianTag {}); \
        if (!ret) \
            return ret.GetError(); \
    } while (false)

Result<void> ParticleConfig::ReadFromHGE(VFS::IStream* stream) noexcept
{
    uint32_t blend = 0;
    READ_CONFIG(blend);
    blend = ((blend >> 16u) & 0x00000003u);
    if (blend & 1)
        IsVertexColorBlendByMultiply = false;
    else
        IsVertexColorBlendByMultiply = true;
    if (blend & 2)
        ColorBlend = ColorBlendMode::Alpha;
    else
        ColorBlend = ColorBlendMode::Add;

    READ_CONFIG(EmissionPerSecond);
    READ_CONFIG(LifeTime);
    READ_CONFIG(ParticleLifeTime);
    READ_CONFIG(Direction);
    READ_CONFIG(Spread);

    uint32_t isRelativeToSpeed = 0;
    READ_CONFIG(isRelativeToSpeed);
    if (isRelativeToSpeed)
        EmitDirection = ParticleEmitDirection::RelativeToSpeed;
    else
        EmitDirection = ParticleEmitDirection::Fixed;

    READ_CONFIG(Speed);
    READ_CONFIG(Gravity);
    READ_CONFIG(RadialAcceleration);
    READ_CONFIG(TangentialAcceleration);
    READ_CONFIG(SizeInitial);
    READ_CONFIG(SizeFinal);
    READ_CONFIG(SizeVariant);
    READ_CONFIG(SpinInitial);
    READ_CONFIG(SpinFinal);
    READ_CONFIG(SpinVariant);
    READ_CONFIG(ColorInitial);
    READ_CONFIG(ColorFinal);
    READ_CONFIG(ColorVariant);
    READ_CONFIG(AlphaVariant);
    return {};
}

#undef READ_CONFIG

#define READ_CONFIG(NAME) \
    Read(NAME, json, #NAME)

void ParticleConfig::ReadFrom(const nlohmann::json& json) noexcept
{
    auto colorBlend = Text::JsonHelper::ReadValue<uint32_t>(json, "ColorBlend");
    if (colorBlend)
        ColorBlend = static_cast<ColorBlendMode>(*colorBlend);
    READ_CONFIG(IsVertexColorBlendByMultiply);

    READ_CONFIG(EmissionPerSecond);
    READ_CONFIG(LifeTime);
    READ_CONFIG(Direction);
    READ_CONFIG(Spread);
    auto emitDirection = Text::JsonHelper::ReadValue<uint32_t>(json, "EmitDirection");
    if (emitDirection)
        EmitDirection = static_cast<ParticleEmitDirection>(*emitDirection);

    READ_CONFIG(ParticleLifeTime);
    READ_CONFIG(Speed);
    READ_CONFIG(Gravity);
    READ_CONFIG(GravityDirection);
    READ_CONFIG(RadialAcceleration);
    READ_CONFIG(TangentialAcceleration);
    READ_CONFIG(SizeInitial);
    READ_CONFIG(SizeFinal);
    READ_CONFIG(SizeVariant);
    READ_CONFIG(SpinInitial);
    READ_CONFIG(SpinFinal);
    READ_CONFIG(SpinVariant);
    READ_CONFIG(ColorInitial);
    READ_CONFIG(ColorFinal);
    READ_CONFIG(ColorVariant);
    READ_CONFIG(AlphaVariant);
}

#undef READ_CONFIG
