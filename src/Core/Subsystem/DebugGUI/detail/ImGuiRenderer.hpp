/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <imgui.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <RefCntAutoPtr.hpp>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>

namespace lstg::Subsystem::DebugGUI::detail
{
    LSTG_DEFINE_EXCEPTION(ImGuiRendererInitializeFailedException);

    /**
     * IMGUI 渲染器
     * @see DiligentTools/Imgui/src/ImGuiDiligentRenderer.hpp
     */
    class ImGuiRenderer
    {
    public:
        ImGuiRenderer(Render::RenderDevice* deviceBridge, ImGuiIO& io);

    public:
        /**
         * 绘制 IMGUI 渲染列表
         * @param drawData 渲染列表
         */
        void RenderDrawData(ImDrawData* drawData) noexcept;

    private:
        void CreateDeviceObjects();
        void SetupRenderState(Diligent::IDeviceContext* context, uint32_t surfaceWidth, uint32_t surfaceHeight) noexcept;

    private:
        Render::RenderDevice* m_pDeviceBridge = nullptr;
        ImGuiIO& m_stImGuiIO;

        bool m_bBaseVertexSupported = false;

        Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pPipelineState;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pVertexConstantBuffer;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pFontTextureSRV;
        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_pShaderResourceBinding;
        Diligent::IShaderResourceVariable* m_pTextureVar = nullptr;

        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pVertexBuffer;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pIndexBuffer;
        size_t m_uVertexBufferSize = 0;  // 顶点数
        size_t m_uIndexBufferSize = 0;  // 索引数
    };
}
