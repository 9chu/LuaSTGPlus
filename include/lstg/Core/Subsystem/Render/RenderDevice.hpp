/**
 * @file
 * @date 2022/3/8
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Exception.hpp>

namespace Diligent
{
    struct IRenderDevice;
    struct IDeviceContext;
    struct ISwapChain;
}

namespace lstg::Subsystem::Render
{
    LSTG_DEFINE_EXCEPTION(RenderDeviceInitializeFailedException);

    /**
     * 渲染设备
     * 由 DiligentEngine 完成抽象，不对应用暴露 DiligentEngine。
     */
    class RenderDevice
    {
    protected:
        RenderDevice() noexcept = default;
        virtual ~RenderDevice() noexcept;

    public:
        /**
         * 获取关联的渲染设备
         */
        [[nodiscard]] Diligent::IRenderDevice* GetDevice() const noexcept;

        /**
         * 获取关联的渲染上下文
         */
        [[nodiscard]] Diligent::IDeviceContext* GetImmediateContext() const noexcept;

        /**
         * 获取关联的交换链
         */
        [[nodiscard]] Diligent::ISwapChain* GetSwapChain() const noexcept;

        /**
         * 是否启用垂直同步
         */
        [[nodiscard]] virtual bool IsVerticalSyncEnabled() const noexcept;

        /**
         * 设置是否启用垂直同步
         */
        virtual void SetVerticalSyncEnabled(bool enable) noexcept;

        /**
         * 获取渲染画面宽度
         * 转发到 SwapChain.GetDesc().Width
         */
        [[nodiscard]] uint32_t GetRenderOutputWidth() const noexcept;

        /**
         * 获取渲染画面高度
         * 转发到 SwapChain.GetDesc().Height
         */
        [[nodiscard]] uint32_t GetRenderOutputHeight() const noexcept;

        /**
         * 开始渲染
         */
        void BeginRender() noexcept;

        /**
         * 完成渲染和进行 Present 动作
         */
        void EndRenderAndPresent() noexcept;

    protected:
        virtual void Present() noexcept;  // 部分平台需要定制的 Present 方法

    protected:
        Diligent::IRenderDevice* m_pRenderDevice = nullptr;
        Diligent::IDeviceContext* m_pRenderContext = nullptr;
        Diligent::ISwapChain* m_pSwapChain = nullptr;
        bool m_bVerticalSync = false;
    };

    using RenderDevicePtr = std::shared_ptr<RenderDevice>;
}
