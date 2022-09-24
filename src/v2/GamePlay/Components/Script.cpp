/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/GamePlay/Components/Script.hpp>

#include <lstg/v2/GamePlay/ScriptObjectPool.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay::Components;

void Script::Reset() noexcept
{
    if (Pool)
        Pool->Free(Pool->GetState(), ScriptObjectId);
    ScriptObjectId = 0;
    Pool = nullptr;
}
