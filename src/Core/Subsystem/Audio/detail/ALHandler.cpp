/**
 * @file
 * @date 2022/8/28
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
