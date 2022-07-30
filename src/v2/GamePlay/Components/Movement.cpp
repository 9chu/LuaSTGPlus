/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GamePlay/Components/Movement.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay::Components;

void Movement::Reset() noexcept
{
    AngularVelocity = 0.;
    Velocity = { 0., 0. };
    AccelVelocity = { 0., 0. };
    RotateToSpeedDirection = false;
}
