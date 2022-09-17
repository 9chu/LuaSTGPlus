/**
 * @file
 * @date 2022/8/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <memory>
#include <al.h>
#include <alc.h>

namespace lstg::Subsystem::Audio::detail
{
    /**
     * OpenAL 设备句柄析构器
     */
    struct ALDeviceHandleCloser
    {
        void operator()(ALCdevice* p) noexcept;
    };

    using ALDeviceHandler = std::unique_ptr<ALCdevice, ALDeviceHandleCloser>;

    /**
     * OpenAL 上下文句柄析构器
     */
    struct ALContextHandleCloser
    {
        void operator()(ALCcontext* p) noexcept;
    };

    using ALContextHandler = std::unique_ptr<ALCcontext, ALContextHandleCloser>;
}
