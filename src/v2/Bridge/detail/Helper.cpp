/**
 * @file
 * @author 9chu
 * @date 2022/6/5
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "Helper.hpp"

#include <lstg/Core/Subsystem/VFS/Path.hpp>

using namespace std;
using namespace lstg::v2::Bridge;
using namespace lstg::v2::Bridge::detail;

std::string detail::ResolveAbsoluteOrRelativePath(Subsystem::Script::LuaStack& stack, std::string_view path) noexcept
{
    // 检查是否采取相对加载规则
    if (path.length() >= 2 && path[0] == '.' && path[1] == '/')
    {
        auto scriptPath = stack.GetTopLevelScriptPath();
        if (scriptPath[0] != '\0')
        {
            Subsystem::VFS::Path p(scriptPath);
            Subsystem::VFS::Path file(string_view{path.data() + 2, path.length() - 2});
            return (p.GetParent() / file).ToString();
        }
    }
    // 默认 fallback 到全路径
    if (path[0] != '/')
        return fmt::format("/{}", path);  // 强制增加一个 '/'
    return string{path};
}
