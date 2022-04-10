/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "ConstantBuffer.hpp"

namespace Diligent
{
    struct ITextureView;
}

namespace lstg::Subsystem
{
    class RenderSystem;
}

namespace lstg::Subsystem::Render
{
    class RenderDevice;

    /**
     * 相机对象
     */
    class Camera
    {
        friend class lstg::Subsystem::RenderSystem;

    public:
        /**
         * 视口
         */
        struct Viewport
        {
            enum {
                VIEWPORT_SIZE_AUTO = 0,  // 设置为 SIZE_AUTO 则自动填充整个交换链缓冲区
            };

            float Left = VIEWPORT_SIZE_AUTO;
            float Top = VIEWPORT_SIZE_AUTO;
            float Width = VIEWPORT_SIZE_AUTO;
            float Height = VIEWPORT_SIZE_AUTO;

            [[nodiscard]] bool operator==(const Viewport& rhs) const noexcept
            {
                return Left == rhs.Left && Top == rhs.Top && Width == rhs.Width && Height == rhs.Height;
            }

            [[nodiscard]] bool IsAutoViewport() const noexcept
            {
                return Left == VIEWPORT_SIZE_AUTO && Top == VIEWPORT_SIZE_AUTO && Width == VIEWPORT_SIZE_AUTO &&
                    Height == VIEWPORT_SIZE_AUTO;
            }
        };

        /**
         * 输出视图
         */
        struct OutputViews
        {
            static constexpr Diligent::ITextureView* kDefaultView = nullptr;  // 设置为 kDefaultView 则使用交换链缓冲区

            Diligent::ITextureView* ColorView = kDefaultView;
            Diligent::ITextureView* DepthStencilView = kDefaultView;

            OutputViews() = default;
            OutputViews(const OutputViews& org);
            OutputViews(OutputViews&& org) noexcept;
            ~OutputViews();

            OutputViews& operator=(const OutputViews& rhs);
            OutputViews& operator=(OutputViews&& rhs) noexcept;

            [[nodiscard]] bool operator==(const OutputViews& rhs) const noexcept
            {
                return ColorView == rhs.ColorView && DepthStencilView == rhs.DepthStencilView;
            }
        };

    public:
        Camera() = default;

    public:
        /**
         * 获取观察矩阵
         */
        const glm::mat4x4& GetViewMatrix() const noexcept { return m_stViewMatrix; }

        /**
         * 设置观察矩阵
         * @param mat 矩阵
         */
        void SetViewMatrix(const glm::mat4x4& mat) noexcept;

        /**
         * 获取投影矩阵
         */
        const glm::mat4x4& GetProjectMatrix() const noexcept { return m_stProjectMatrix; }

        /**
         * 设置投影矩阵
         * @param mat 矩阵
         */
        void SetProjectMatrix(const glm::mat4x4& mat) noexcept;

        /**
         * 获取投影x观察矩阵
         */
        const glm::mat4x4& GetProjectViewMatrix() const noexcept;

        /**
         * 获取渲染目标
         */
        [[nodiscard]] const OutputViews& GetOutputViews() const noexcept { return m_stOutputViews; }

        /**
         * 设置渲染目标
         * @param views 目标
         */
        void SetOutputViews(const OutputViews& views) noexcept;

        /**
         * 获取视口
         */
        const Viewport& GetViewport() const noexcept { return m_stViewport; }

        /**
         * 设置视口
         */
        void SetViewport(Viewport vp) noexcept;

    private:
        glm::mat4x4 m_stViewMatrix;
        glm::mat4x4 m_stProjectMatrix;
        mutable std::optional<glm::mat4x4> m_stProjectViewMatrix;
        OutputViews m_stOutputViews;
        Viewport m_stViewport;

        // Dirty flags
        bool m_bStateDirty = true;
    };

    using CameraPtr = std::shared_ptr<Camera>;
}
