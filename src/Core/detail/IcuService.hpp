/**
 * @file
 * @date 2022/6/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <unicode/utypes.h>
#include <unicode/udata.h>
#include <unicode/ubidi.h>
#include <unicode/brkiter.h>
#include <lstg/Core/Result.hpp>

namespace lstg::detail
{
    struct IcuBidiDeleter
    {
        void operator()(UBiDi* bidi) noexcept;
    };

    using IcuBidiPtr = std::unique_ptr<UBiDi, IcuBidiDeleter>;
    using IcuBreakIteratorPtr = std::unique_ptr<icu::BreakIterator>;

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
        /**
         * 创建 BiDi 对象
         */
        Result<IcuBidiPtr> CreateBidi() noexcept;

        /**
         * 创建 Grapheme 分段器
         */
        Result<IcuBreakIteratorPtr> CreateGraphemeBreakIterator() noexcept;
    };
}
