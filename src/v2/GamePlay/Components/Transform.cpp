/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GamePlay/Components/Transform.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay::Components;

void Transform::Reset() noexcept
{
    Location = { 0., 0. };
    LastLocation = { 0., 0. };
    LocationDelta = { 0., 0. };
    Rotation = 0.;
}
