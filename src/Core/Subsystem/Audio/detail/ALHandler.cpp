/**
 * @file
 * @date 2022/8/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "ALHandler.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio::detail;

// <editor-fold desc="ALDeviceHandleCloser">

void ALDeviceHandleCloser::operator()(ALCdevice* p) noexcept
{
    if (p)
        ::alcCloseDevice(p);
}

// </editor-fold>

// <editor-fold desc="ALContextHandleCloser">

void ALContextHandleCloser::operator()(ALCcontext* p) noexcept
{
    if (p)
        ::alcDestroyContext(p);
}

// </editor-fold>
