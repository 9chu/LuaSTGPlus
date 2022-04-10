/**
 * @file
 * @date 2022/3/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Camera.hpp>

#include <glm/ext.hpp>
#include <TextureView.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

// <editor-fold desc="Camera::OutputViews">

Camera::OutputViews::OutputViews(const OutputViews& org)
    : ColorView(org.ColorView), DepthStencilView(org.DepthStencilView)
{
    if (ColorView)
        ColorView->AddRef();
    if (DepthStencilView)
        DepthStencilView->AddRef();
}

Camera::OutputViews::OutputViews(OutputViews&& org) noexcept
{
    std::swap(ColorView, org.ColorView);
    std::swap(DepthStencilView, org.DepthStencilView);
}

Camera::OutputViews::~OutputViews()
{
    if (ColorView)
    {
        ColorView->Release();
        ColorView = nullptr;
    }
    if (DepthStencilView)
    {
        DepthStencilView->Release();
        DepthStencilView = nullptr;
    }
}

Camera::OutputViews& Camera::OutputViews::operator=(const OutputViews& rhs)
{
    if (this == &rhs)
        return *this;

    if (ColorView)
        ColorView->Release();
    if (DepthStencilView)
        DepthStencilView->Release();

    ColorView = rhs.ColorView;
    DepthStencilView = rhs.DepthStencilView;

    if (ColorView)
        ColorView->AddRef();
    if (DepthStencilView)
        DepthStencilView->AddRef();
    return *this;
}

Camera::OutputViews& Camera::OutputViews::operator=(OutputViews&& rhs) noexcept
{
    std::swap(ColorView, rhs.ColorView);
    std::swap(DepthStencilView, rhs.DepthStencilView);
    return *this;
}

// </editor-fold>
// <editor-fold desc="Camera">

Camera::Camera()
    : m_stViewMatrix(glm::identity<glm::mat4x4>()), m_stProjectMatrix(glm::identity<glm::mat4x4>()),
    m_stProjectViewMatrix(glm::identity<glm::mat4x4>())
{

}

void Camera::SetViewMatrix(const glm::mat4x4& mat) noexcept
{
    m_stViewMatrix = mat;
    m_stProjectViewMatrix.reset();
    m_bStateDirty = true;
}

void Camera::SetProjectMatrix(const glm::mat4x4& mat) noexcept
{
    m_stProjectMatrix = mat;
    m_stProjectViewMatrix.reset();
    m_bStateDirty = true;
}

const glm::mat4x4& Camera::GetProjectViewMatrix() const noexcept
{
    if (!m_stProjectViewMatrix)
        m_stProjectViewMatrix = m_stProjectMatrix * m_stViewMatrix;
    return *m_stProjectViewMatrix;
}

void Camera::SetViewport(Viewport vp) noexcept
{
    m_stViewport = vp;
    m_bStateDirty = true;
}

void Camera::SetOutputViews(const OutputViews& views) noexcept
{
    m_stOutputViews = views;
    m_bStateDirty = true;
}

// </editor-fold>
