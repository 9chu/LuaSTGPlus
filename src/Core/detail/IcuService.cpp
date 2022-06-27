/**
 * @file
 * @date 2022/6/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "IcuService.hpp"

#include <cassert>
#include <unicode/uvernum.h>

using namespace std;
using namespace lstg;
using namespace lstg::detail;

// ${CMAKE_BINARY_DIR}/icudata/icudata.cpp
extern const uint8_t kIcuDataContent[];

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
