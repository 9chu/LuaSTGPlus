/**
 * @file
 * @date 2022/6/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "IcuService.hpp"

#include <cassert>
#include <unicode/uvernum.h>
#include "IcuError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::detail;

// ${CMAKE_BINARY_DIR}/icudata/icudata.cpp
extern const uint8_t kIcuDataContent[];

// <editor-fold desc="IcuBidiDeleter">

void IcuBidiDeleter::operator()(UBiDi* bidi) noexcept
{
    ::ubidi_close(bidi);
}

// </editor-fold>

IcuService& IcuService::GetInstance() noexcept
{
    static IcuService kInstance;
    return kInstance;
}

IcuService::IcuService() noexcept
{
    char packageName[32];
    ::sprintf(packageName, "icudt%d%s", U_ICU_VERSION_MAJOR_NUM, U_IS_BIG_ENDIAN ? "b" : "l");

    UErrorCode status = U_ZERO_ERROR;
    ::udata_setAppData(packageName, kIcuDataContent, &status);
    assert(U_SUCCESS(status));
}

Result<IcuBidiPtr> IcuService::CreateBidi() noexcept
{
    // 创建 UBiDi
    auto bidi = ::ubidi_open();
    if (!bidi)
        return make_error_code(errc::not_enough_memory);
    return IcuBidiPtr(bidi);
}

Result<IcuBreakIteratorPtr> IcuService::CreateGraphemeBreakIterator() noexcept
{
    // 创建字符切分器
    try
    {
        UErrorCode status = U_ZERO_ERROR;
        auto inst = icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), status);
        if (U_FAILURE(status))
            return make_error_code(status);
        return IcuBreakIteratorPtr(inst);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}
