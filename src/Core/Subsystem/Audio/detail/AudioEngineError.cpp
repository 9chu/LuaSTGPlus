/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
