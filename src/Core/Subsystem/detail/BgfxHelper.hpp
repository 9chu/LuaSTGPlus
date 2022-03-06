/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

namespace lstg::Subsystem::detail
{
    inline const char* ToString(bgfx::RendererType::Enum t) noexcept
    {
        switch (t)
        {
            case bgfx::RendererType::Noop:
                return "NoRendering";
            case bgfx::RendererType::Agc:
                return "AGC";
            case bgfx::RendererType::Direct3D9:
                return "D3D9";
            case bgfx::RendererType::Direct3D11:
                return "D3D11";
            case bgfx::RendererType::Direct3D12:
                return "D3D12";
            case bgfx::RendererType::Gnm:
                return "GNM";
            case bgfx::RendererType::Metal:
                return "Metal";
            case bgfx::RendererType::Nvn:
                return "NVN";
            case bgfx::RendererType::OpenGLES:
                return "OpenGLES2+";
            case bgfx::RendererType::OpenGL:
                return "OpenGL2.1+";
            case bgfx::RendererType::Vulkan:
                return "Vulkan";
            case bgfx::RendererType::WebGPU:
                return "WebGPU";
            default:
                return "<unknown>";
        }
    }
}
