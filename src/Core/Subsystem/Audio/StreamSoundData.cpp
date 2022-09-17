/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
        return make_shared<SDLSoundDecoder>(m_pStream);
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
