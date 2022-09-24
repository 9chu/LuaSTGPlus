/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
