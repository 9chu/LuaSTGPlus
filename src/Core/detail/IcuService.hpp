/**
 * @file
 * @date 2022/6/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <unicode/udata.h>

namespace lstg::detail
{
    /**
     * ICU 服务单例
     * 用于初始化 ICU 数据文件。
     */
    class IcuService
    {
    public:
        static IcuService& GetInstance() noexcept;

    protected:
        IcuService() noexcept;
        IcuService(const IcuService&) = delete;
        IcuService(IcuService&&) noexcept = delete;

    public:
        // TODO
    };
}
