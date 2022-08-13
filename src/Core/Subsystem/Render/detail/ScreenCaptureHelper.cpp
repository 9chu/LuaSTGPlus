/**
 * @file
 * @date 2022/8/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "ScreenCaptureHelper.hpp"

#include <RenderDevice.h>
#include <SwapChain.h>
#include <DeviceContext.h>
#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail;

using namespace Diligent;

LSTG_DEF_LOG_CATEGORY(ScreenCaptureHelper);

ScreenCaptureHelper::ScreenCaptureHelper(RenderDevice* device)
    : m_pDevice(device)
{
    FenceDesc desc;
    desc.Name = "Screen capture fence";
    m_pDevice->GetDevice()->CreateFence(desc, &m_pFence);
    if (!m_pFence)
        LSTG_THROW(ScreenCaptureHelperInitializeFailedException, "Create fence fail");
}

Result<void> ScreenCaptureHelper::AddCaptureTask(ScreenCaptureCallback callback, bool clearAlpha) noexcept
{
    try
    {
        PendingTask task;
        task.Callback = std::move(callback);
        task.ClearAlpha = clearAlpha;
        m_stPendingTasks.emplace_back(std::move(task));
        return {};
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

void ScreenCaptureHelper::ProcessCaptureTasks() noexcept
{
    // 检查所有等待的任务，发起截图操作
    for (auto it = m_stPendingTasks.begin(); it != m_stPendingTasks.end(); )
    {
        auto& task = (*it);

        if (!task.Committed)
        {
            // 发起截图操作
            auto* currentRTV = m_pDevice->GetSwapChain()->GetCurrentBackBufferRTV();
            assert(currentRTV);
            auto* currentRT = currentRTV->GetTexture();
            const auto& currentRTDesc = currentRT->GetDesc();

            // Step1. 申请 Staging Texture
            auto tex = AllocStagingTexture(currentRTDesc);
            if (!tex)
            {
                try
                {
                    task.Callback(make_error_code(errc::not_enough_memory));
                }
                catch (...)
                {
                    assert(false);
                }
                it = m_stPendingTasks.erase(it);
                continue;
            }
            else
            {
                // Step2. 发起 Copy 操作
                auto expectedFenceId = m_ullCurrentFenceId++;
                auto frameId = m_pDevice->GetPresentedFrameCount();  // 由于我们总是在 Present 之前调用，此时 FrameCount 恰好是 FrameId

                CopyTextureAttribs copyDesc(currentRT, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, tex,
                    RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                m_pDevice->GetImmediateContext()->CopyTexture(copyDesc);
                m_pDevice->GetImmediateContext()->EnqueueSignal(m_pFence, expectedFenceId);

                task.Committed = true;
                task.Texture = tex;
                task.FenceId = expectedFenceId;
                task.FrameId = frameId;
            }
        }

        ++it;
    }

    // 处理等待的任务，收取截图
    if (!m_stPendingTasks.empty())
    {
        auto completedFenceValue = m_pFence->GetCompletedValue();

        for (auto it = m_stPendingTasks.begin(); it != m_stPendingTasks.end(); )
        {
            auto& task = (*it);
            assert(task.Committed);  // 到这里，应该所有的任务都被提交了

            if (task.FenceId <= completedFenceValue)
            {
                // Step1. 从纹理拷贝数据
                assert(task.Texture);
                MappedTextureSubresource texData;
                m_pDevice->GetImmediateContext()->MapTextureSubresource(task.Texture, 0, 0, MAP_READ, MAP_FLAG_DO_NOT_WAIT, nullptr,
                    texData);
                auto copyRet = CopyFromTexture(task.Texture->GetDesc(), texData, task.ClearAlpha);
                m_pDevice->GetImmediateContext()->UnmapTextureSubresource(task.Texture, 0, 0);

                // Step2. 调用回调方法
                try
                {
                    if (!copyRet)
                    {
                        task.Callback(copyRet.GetError());
                    }
                    else
                    {
                        assert(m_stTmpTextureData);
                        task.Callback(&*m_stTmpTextureData);
                    }
                }
                catch (...)
                {
                    assert(false);
                }

                // Step3. 回收纹理
                try
                {
                    m_stAvailableTextures.emplace_back(std::move(task.Texture));
                }
                catch (...)  // bad_alloc
                {
                }

                it = m_stPendingTasks.erase(it);
                continue;
            }

            ++it;
        }
    }
}

Diligent::RefCntAutoPtr<Diligent::ITexture> ScreenCaptureHelper::AllocStagingTexture(const Diligent::TextureDesc& desc) noexcept
{
    RefCntAutoPtr<ITexture> ret;

    // 取一个空闲的纹理
    while (!m_stAvailableTextures.empty() && !ret)
    {
        ret = std::move(m_stAvailableTextures.back());
        m_stAvailableTextures.pop_back();

        const auto& cached = ret->GetDesc();
        if (!(cached.Width == desc.Width && cached.Height == desc.Height && cached.Format == desc.Format))
            ret.Release();
    }

    // 没有合适的，创建一个新的纹理
    if (!ret)
    {
        TextureDesc texDesc;
        texDesc.Name = "Staging texture for screen capture";
        texDesc.Type = RESOURCE_DIM_TEX_2D;
        texDesc.Width = desc.Width;
        texDesc.Height = desc.Height;
        texDesc.Format = desc.Format;
        texDesc.Usage = USAGE_STAGING;
        texDesc.CPUAccessFlags = CPU_ACCESS_READ;
        m_pDevice->GetDevice()->CreateTexture(texDesc, nullptr, &ret);
        if (!ret)
            LSTG_LOG_ERROR_CAT(ScreenCaptureHelper, "CreateTexture fail");
    }

    return ret;
}

Result<void> ScreenCaptureHelper::CopyFromTexture(const Diligent::TextureDesc& desc, const Diligent::MappedTextureSubresource& res,
    bool clearAlpha) noexcept
{
    assert(desc.Is2D());
    assert(!desc.IsArray());

    auto inputWidth = desc.Width;
    auto inputHeight = desc.Height;
    auto inputFormat = desc.Format;
    auto inputBuffer = reinterpret_cast<const uint8_t*>(res.pData);
    auto inputBufferStride = res.Stride;

    // 决定格式
    Texture2DFormats outputFormat;
    switch (inputFormat)
    {
        case Diligent::TEX_FORMAT_R8_UNORM:
            outputFormat = Texture2DFormats::R8;
            break;
        case Diligent::TEX_FORMAT_RG8_UNORM:
            outputFormat = Texture2DFormats::R8G8;
            break;
        case Diligent::TEX_FORMAT_BGRA8_UNORM:
        case Diligent::TEX_FORMAT_RGBA8_UNORM:
            outputFormat = Texture2DFormats::R8G8B8A8;
            break;
        case Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB:
        case Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB:
            outputFormat = Texture2DFormats::R8G8B8A8_SRGB;
            break;
        case Diligent::TEX_FORMAT_R16_UNORM:
            outputFormat = Texture2DFormats::R16;
            break;
        case Diligent::TEX_FORMAT_RG16_UNORM:
            outputFormat = Texture2DFormats::R16G16;
            break;
        case Diligent::TEX_FORMAT_RGBA16_UNORM:
            outputFormat = Texture2DFormats::R16G16B16A16;
            break;
        default:
            LSTG_LOG_ERROR_CAT(ScreenCaptureHelper, "Unknown back buffer format {}", desc.Format);
            return make_error_code(errc::invalid_argument);
    }

    // 创建纹理数据
    if (!m_stTmpTextureData || m_stTmpTextureData->GetWidth() != inputWidth || m_stTmpTextureData->GetHeight() != inputHeight ||
        m_stTmpTextureData->GetFormat() != outputFormat)
    {
        try
        {
            m_stTmpTextureData.emplace(inputWidth, inputHeight, outputFormat);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    assert(m_stTmpTextureData && m_stTmpTextureData->GetWidth() == inputWidth && m_stTmpTextureData->GetHeight() == inputHeight &&
        m_stTmpTextureData->GetFormat() == outputFormat);

    // 发起拷贝
    try
    {
        auto inputScanLine = inputBuffer;
        auto outputScanLine = m_stTmpTextureData->GetBuffer().GetData();
        auto outputBufferStride = m_stTmpTextureData->GetStride();
        for (uint32_t h = 0; h < desc.Height; ++h)
        {
            ::memcpy(outputScanLine, inputScanLine, std::min<size_t>(inputBufferStride, outputBufferStride));

            // 特殊处理 BGRA -> RGBA
            if (inputFormat == TEX_FORMAT_BGRA8_UNORM || inputFormat == TEX_FORMAT_BGRA8_UNORM_SRGB)
            {
                for (uint32_t w = 0; w < desc.Width; ++w)
                    std::swap(outputScanLine[w * 4 + 0], outputScanLine[w * 4 + 2]);
            }

            // 特殊处理 清理 alpha 通道
            if (outputFormat == Texture2DFormats::R8G8B8A8 || outputFormat == Texture2DFormats::R8G8B8A8_SRGB)
            {
                for (uint32_t w = 0; w < desc.Width; ++w)
                    outputScanLine[w * 4 + 3] = 0xFF;
            }
            else if (outputFormat == Texture2DFormats::R16G16B16A16)
            {
                for (uint32_t w = 0; w < desc.Width; ++w)
                    outputScanLine[w * 8 + 6] = outputScanLine[w * 8 + 7] = 0xFF;
            }

            inputScanLine += inputBufferStride;
            outputScanLine += outputBufferStride;
        }
        return {};
    }
    catch (...)  // bad_alloc
    {
        LSTG_LOG_ERROR_CAT(ScreenCaptureHelper, "Create texture2d data fail, width={}, height={}", desc.Width, desc.Height);
        return make_error_code(errc::not_enough_memory);
    }
}
