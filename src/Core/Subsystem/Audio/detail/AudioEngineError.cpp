/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "AudioEngineError.hpp"

#include <cassert>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio::detail;

const AudioEngineErrorCategory& AudioEngineErrorCategory::GetInstance() noexcept
{
   static const AudioEngineErrorCategory kInstance;
   return kInstance;
}

const char* AudioEngineErrorCategory::name() const noexcept
{
   return "AudioEngineError";
}

std::string AudioEngineErrorCategory::message(int ev) const
{
   switch (static_cast<AudioEngineErrorCodes>(ev))
   {
       case AudioEngineErrorCodes::Ok:
           return "Ok";
       default:
           assert(false);
           return "<unknown>";
   }
}
