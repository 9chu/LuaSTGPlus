/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Bridge/MiscModule.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/RenderSystem.hpp>
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>
#include "detail/Helper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

LSTG_DEF_LOG_CATEGORY(MiscModule);

void MiscModule::Registry()
{
    LSTG_LOG_DEPRECATED(MiscModule, Registry);
}

void MiscModule::CaptureSnapshot(const char* path)
{
    auto& app = detail::GetGlobalApp();
    auto& renderSystem = *app.GetSubsystem<Subsystem::RenderSystem>();

    // 构造文件名
    Subsystem::VFS::Path savePathBase {"/storage"};
    Subsystem::VFS::Path savePath {path};
    savePath = savePathBase / savePath;

    // 发起截图操作
    renderSystem.CaptureScreen([savePath](Result<const Subsystem::Render::Texture2DData*> data) {
        if (!data)
        {
            LSTG_LOG_ERROR_CAT(MiscModule, "Capture screen fail: {}", data.GetError());
            return;
        }

        // 打开数据流
        auto& app = detail::GetGlobalApp();
        auto& vfs = *app.GetSubsystem<Subsystem::VirtualFileSystem>();
        auto stream = vfs.OpenFile(savePath.ToStringView(), Subsystem::VFS::FileAccessMode::Write, Subsystem::VFS::FileOpenFlags::Truncate);
        if (!stream)
        {
            LSTG_LOG_ERROR_CAT(MiscModule, "Open screenshot file '{}' fail: {}", savePath.ToStringView(), stream.GetError());
            return;
        }

        // 写出截图
        auto texData = *data;
        assert(texData);
        auto ret = Subsystem::Render::SaveToPng(stream->get(), texData);
        if (!ret)
            LSTG_LOG_ERROR_CAT(MiscModule, "Write screenshot fail: {}", ret.GetError());
        else
            LSTG_LOG_INFO_CAT(MiscModule, "Screenshot save to '{}'", savePath.ToStringView());
    });
}

bool MiscModule::Execute(const char* path, std::optional<std::string_view> arguments, std::optional<const char*> directory,
    std::optional<bool> wait /* =true */)
{
    // NOTE: 出于跨平台限定的考虑，不再提供 Execute 的支持
    LSTG_LOG_DEPRECATED(MiscModule, Execute);
    return false;
}

LSTGRandomizer MiscModule::NewRandomizer()
{
    LSTGRandomizer ret {};
    ret.SetSeed(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
    return ret;
}

LSTGColor MiscModule::NewColor(Subsystem::Script::LuaStack& stack)
{
    LSTGColor ret {};
    if (stack.GetTop() == 1)
    {
        auto argb = static_cast<uint32_t>(luaL_checknumber(stack, 1));  // 使用 checkinteger 会在 lua51 下得到意外结果
        ret.r((argb & 0x00FF0000u) >> 16u);
        ret.g((argb & 0x0000FF00u) >> 8u);
        ret.b(argb & 0x000000FFu);
        ret.a((argb & 0xFF000000u) >> 24u);
        return ret;
    }

    auto a = luaL_checkinteger(stack, 1);
    auto r = luaL_checkinteger(stack, 2);
    auto g = luaL_checkinteger(stack, 3);
    auto b = luaL_checkinteger(stack, 4);
    ret.r(std::clamp<int>(static_cast<int>(r), 0, 255));
    ret.g(std::clamp<int>(static_cast<int>(g), 0, 255));
    ret.b(std::clamp<int>(static_cast<int>(b), 0, 255));
    ret.a(std::clamp<int>(static_cast<int>(a), 0, 255));
    return ret;
}

LSTGBentLaserData MiscModule::NewBentLaserData()
{
    return {};
}
