/**
 * @file
 * @date 2022/8/28
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <memory>

#ifdef LSTG_PLATFORM_EMSCRIPTEN
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif

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
