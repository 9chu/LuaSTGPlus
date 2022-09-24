/**
* @file
* @date 2022/8/19
* @author 9chu
* 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <Buffer.h>
#include <Texture.h>
#include <PipelineState.h>
#include <RefCntAutoPtr.hpp>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>
#include <lstg/Core/Subsystem/Render/ColorRGBA32.hpp>
#include <lstg/Core/Subsystem/Render/Mesh.hpp>

namespace lstg::Subsystem::Render::detail
{
    LSTG_DEFINE_EXCEPTION(GammaCorrectHelperInitializeFailedException);

    /**
     * 用于实现 Gamma 校准的后处理过程
     */
    class GammaCorrectHelper
    {
    public:
        GammaCorrectHelper(RenderDevice* device);

    public:
        /**
         * 获取捕获渲染用的 Frame 缓冲区
         */
        Diligent::ITextureView* GetRenderTargetView() noexcept { return m_pColorFrameBufferRTV; };

        /**
         * 重新缩放 Frame 缓冲区
         */
        void ResizeFrameBuffer() noexcept;

        /**
         * 绘制 Frame 并完成 Gamma 校准
         */
        void DrawFrameBuffer() noexcept;

    private:
        RenderDevice* m_pDevice = nullptr;

        Diligent::RefCntAutoPtr<Diligent::ITexture> m_pColorFrameBuffer;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pColorFrameBufferSRV;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pColorFrameBufferRTV;
        Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pPostEffectPSO;
        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_pPostEffectSRB;
        Diligent::IShaderResourceVariable* m_pColorFrameTextureVar = nullptr;

        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pVertexBuffer;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pIndexBuffer;
    };
}
