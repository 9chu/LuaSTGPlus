/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/GameObjectModule.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

Subsystem::Script::LuaStack::AbsIndex GameObjectModule::GetObjectTable()
{
    // TODO
//			return LPOOL.GetObjectTable(L);
    return Subsystem::Script::LuaStack::AbsIndex { 0u };
}
