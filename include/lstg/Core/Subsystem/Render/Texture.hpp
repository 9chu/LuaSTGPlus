/**
 * @file
 * @date 2022/4/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <memory>

namespace Diligent
{
    struct ITexture;
}

namespace lstg::Subsystem
{
    class RenderSystem;
}

namespace lstg::Subsystem::Render
{
    class Material;
}

namespace lstg::Subsystem::Render
{
    /**
     * 纹理
     */
    class Texture
    {
        friend class lstg::Subsystem::Render::Material;
        friend class lstg::Subsystem::RenderSystem;

    public:
        Texture(Diligent::ITexture* handler);
        Texture(const Texture&) = delete;
        Texture(Texture&&) noexcept = delete;
        ~Texture();

    private:
        Diligent::ITexture* m_pNativeHandler = nullptr;
    };

    using TexturePtr = std::shared_ptr<Texture>;
}
