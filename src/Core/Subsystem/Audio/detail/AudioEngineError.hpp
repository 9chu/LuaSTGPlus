/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <system_error>

namespace lstg::Subsystem::Audio::detail
{
   /**
    * 音频引擎错误码
    */
   enum class AudioEngineErrorCodes
   {
       Ok = 0,
       BusChannelCircularSendingDetected = 1,
       NoSoundSourceAvailable = 2,
       SoundSourceAlreadyDisposed = 3,
   };

   /**
    * 音频引擎错误代码分类
    */
   class AudioEngineErrorCategory :
       public std::error_category
   {
   public:
       static const AudioEngineErrorCategory& GetInstance() noexcept;

   public:
       const char* name() const noexcept override;
       std::string message(int ev) const override;
   };

   inline std::error_code make_error_code(AudioEngineErrorCodes ec) noexcept
   {
       return { static_cast<int>(ec), AudioEngineErrorCategory::GetInstance() };
   }
}

template <>
struct std::is_error_code_enum<lstg::Subsystem::Audio::detail::AudioEngineErrorCodes> : std::true_type {};
