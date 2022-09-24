/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "StreamSoundData.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio;

StreamSoundData::StreamSoundData(VFS::StreamPtr stream)
    : m_pStream(std::move(stream))
{
}

Result<SoundDecoderPtr> StreamSoundData::CreateDecoder() noexcept
{
    try
    {
        // 由于 SoundDecoder 在单独线程处理，且需要保证互不干扰，此时需要 Clone Stream
        auto clone = m_pStream->Clone();
        if (!clone)
            return clone.GetError();
        return make_shared<SDLSoundDecoder>(std::move(*clone));
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<SoundDataPtr> Subsystem::Audio::CreateStreamSoundData(VFS::StreamPtr stream) noexcept
{
    try
    {
        assert(stream);

        auto seekableStream = ConvertToSeekableStream(std::move(stream));
        if (!seekableStream)
            return seekableStream.GetError();

        return make_shared<StreamSoundData>(std::move(*seekableStream));
    }
    catch (const std::system_error& ex)
    {
        return ex.code();
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}
