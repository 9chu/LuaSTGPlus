/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/LSTGBentLaserData.hpp>

#include <lstg/v2/GamePlay/ScriptObjectPool.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

void LSTGBentLaserData::Update(LuaStack& stack, AbsIndex baseObject, int32_t length, uint32_t width)
{
    assert(baseObject == 2);

    // 检查参数
    if (stack.TypeOf(baseObject) != LUA_TTABLE)
        stack.Error("invalid argument #1, luastg object required for 'Update'.");
    stack.RawGet(baseObject, GamePlay::kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<GamePlay::ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    if (!m_stImplementation.Update(id, length, width))
        stack.Error("invalid lstg object for 'Update'.");
}

void LSTGBentLaserData::Release()
{
}

void LSTGBentLaserData::Render(LuaStack& stack, const char* texture, const char* blend, LSTGColor* color, double texLeft, double texTop,
    double texWidth, double texHeight, std::optional<double> scale /* =1 */) const
{
    Math::UVRectangle rect {
        static_cast<float>(texLeft), static_cast<float>(texTop), static_cast<float>(texWidth), static_cast<float>(texHeight)
    };
    if (!m_stImplementation.Render(texture, v2::BlendMode(blend), *color, rect, scale ? *scale : 1.))
        stack.Error("can't render object with texture '%s'.", texture);
}

bool LSTGBentLaserData::CollisionCheck(double x, double y, std::optional<double> rot /* =0 */, std::optional<double> a /* =0 */,
    std::optional<double> b /* =0 */, std::optional<bool> rect /* =false */) const
{
    return m_stImplementation.CollisionCheck(x, y, rot ? *rot : 0., a ? *a : 0., b ? *b : 0., rect ? *rect : false);
}

bool LSTGBentLaserData::BoundCheck() const
{
    return m_stImplementation.BoundCheck();
}

std::string LSTGBentLaserData::ToString() const
{
    return "lstgBentLaserData";
}
