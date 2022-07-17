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
