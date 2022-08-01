/**
* @file
* @date 2022/8/1
* @author 9chu
* 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
*/
#pragma once
#include <Buffer.h>
#include <PipelineState.h>
#include <RefCntAutoPtr.hpp>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>
#include <lstg/Core/Subsystem/Render/ColorRGBA32.hpp>

namespace lstg::Subsystem::Render::detail
{
    LSTG_DEFINE_EXCEPTION(ClearHelperInitializeFailedException);

    /**
     * 用于实现 Viewport 内的 Clear 操作
     */
    class ClearHelper
    {
    public:
        ClearHelper(RenderDevice* device);

    public:
        /**
         * 仅清除颜色
         * @param color 颜色
         */
        void ClearColor(ColorRGBA32 color) noexcept;

        /**
         * 仅清除深度信息
         * @param depth 深度
         */
        void ClearDepth(float depth) noexcept;

        /**
         * 清除颜色和深度
         * @param color 颜色
         * @param depth 深度
         */
        void ClearDepthColor(ColorRGBA32 color, float depth) noexcept;

    private:
        RenderDevice* m_pDevice = nullptr;

        Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pColorDepthClearPSO;  // 清理颜色+深度信息
        Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pColorOnlyClearPSO;  // 只清理颜色信息
        Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pDepthOnlyClearPSO;  // 只清理深度信息
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pVertexBuffer;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pIndexBuffer;
    };
}
