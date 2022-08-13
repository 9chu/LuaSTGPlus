/**
 * @file
 * @date 2022/8/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <vector>
#include <functional>
#include <optional>
#include <Fence.h>
#include <Texture.h>
#include <RefCntAutoPtr.hpp>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>
#include <lstg/Core/Subsystem/Render/Texture2DData.hpp>

namespace lstg::Subsystem::Render::detail
{
    LSTG_DEFINE_EXCEPTION(ScreenCaptureHelperInitializeFailedException);

    using ScreenCaptureCallback = std::function<void(Result<const Texture2DData*>)>;

    /**
     * 截图工具
     */
    class ScreenCaptureHelper
    {
    public:
        ScreenCaptureHelper(RenderDevice* device);

    public:
        /**
         * 添加截取屏幕任务
         * @param callback 回调
         * @param clearAlpha 是否清理 Alpha 通道
         */
        Result<void> AddCaptureTask(ScreenCaptureCallback callback, bool clearAlpha) noexcept;

        /**
         * 处理截屏任务
         */
        void ProcessCaptureTasks() noexcept;

    private:
        Diligent::RefCntAutoPtr<Diligent::ITexture> AllocStagingTexture(const Diligent::TextureDesc& desc) noexcept;
        Result<void> CopyFromTexture(const Diligent::TextureDesc& desc, const Diligent::MappedTextureSubresource& res,
            bool clearAlpha) noexcept;

    private:
        struct PendingTask
        {
            bool Committed = false;
            Diligent::RefCntAutoPtr<Diligent::ITexture> Texture;
            uint64_t FenceId = 0ull;
            uint32_t FrameId = 0u;
            ScreenCaptureCallback Callback;
            bool ClearAlpha = true;
        };

        RenderDevice* m_pDevice = nullptr;

        Diligent::RefCntAutoPtr<Diligent::IFence> m_pFence;
        uint64_t m_ullCurrentFenceId = 1ull;  // 初始值是0，从1开始提交

        // 可复用的 Staging Texture
        std::vector<Diligent::RefCntAutoPtr<Diligent::ITexture>> m_stAvailableTextures;

        // 等待的任务
        std::vector<PendingTask> m_stPendingTasks;

        // 临时纹理对象
        std::optional<Texture2DData> m_stTmpTextureData;
    };
}
