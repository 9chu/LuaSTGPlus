/**
 * @file
 * @date 2022/4/12
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Texture.hpp>

#include <Texture.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

Texture::Texture(Diligent::ITexture* handler)
    : m_pNativeHandler(handler)
{
    assert(handler);
    m_pNativeHandler->AddRef();
}

Texture::~Texture()
{
    if (m_pNativeHandler)
    {
        m_pNativeHandler->Release();
        m_pNativeHandler = nullptr;
    }
}
