/**
 * @file
 * @date 2022/3/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Script/SandBox.hpp>

#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Script;

SandBox::SandBox(LuaStack mainThread, std::string_view baseDirectory)
    : m_stMainThread(std::move(mainThread)), m_stBaseDirectory(baseDirectory)
{
#ifdef LSTG_SHIPPING
    m_dCheckInterval = 0.;
#else
    m_dCheckInterval = 1.;  // 1秒
#endif
}

Result<void> SandBox::ImportScript(std::string_view path) noexcept
{
    // TODO
    return {};
}

void SandBox::Update(double elapsedTime) noexcept
{
    // TODO
}

Result<LuaReference> SandBox::ModuleLocator(const char* modname) noexcept
{
#if 0
    auto it = m_stFiles.find(name);
    if (it == m_stFiles.end())
    {
        string fullPath;

        struct stat fileStat;
        if (::stat(name, &fileStat) != 0)
        {
            fullPath = m_stBaseDir + name;

            if (::stat(fullPath.c_str(), &fileStat) != 0)
                MOE_THROW(ApiException, "Stat file \"{0}\" error", name);
        }
        else
            fullPath = name;

        MOE_LOG_INFO("Loading script: {0}", fullPath);

        LoadedFile node;
        node.Path = std::move(fullPath);
        node.LastModify = fileStat.st_mtime;

        {
            LuaWrapper::StackBalancer balancer(m_stState);

            // 创建环境
            m_stState.NewTable();  // t

            m_stState.NewTable();  // t t
            m_stState.Push("__index");  // t t __index
            m_stState.GetGlobal("_G");  // t t __index _G
            m_stState.RawSet(-3);  // t t
            lua_setmetatable(m_stState, -2);  // t

            auto env = LuaWrapper::Reference::Capture(m_stState);

            // 创建环境透传函数
            m_stState.Push(function<LuaWrapper::Reference()>([=]() { return env; }));

            node.Env = env;
            node.TransparentFunc = LuaWrapper::Reference::Capture(m_stState);
        }

        auto jt = m_stFiles.emplace(name, std::move(node));
        ExecFile(jt.first->second);

        return jt.first->second.TransparentFunc;
    }

    return it->second.TransparentFunc;
#endif
    return make_error_code(errc::not_supported);
}
