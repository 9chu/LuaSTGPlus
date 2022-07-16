/**
 * @file
 * @date 2022/7/10
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/VFS/IStream.hpp>

#include <vector>
#include <lstg/Core/Subsystem/VFS/ContainerStream.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

Result<Subsystem::VFS::StreamPtr> Subsystem::VFS::ConvertToSeekableStream(Subsystem::VFS::StreamPtr stream) noexcept
{
    // 如果本身就是可以 Seek 的，就忽略
    if (stream->IsSeekable())
        return stream;

    // 预分配
    Subsystem::VFS::MemoryStreamPtr ret;
    try
    {
        ret = make_shared<Subsystem::VFS::MemoryStream>();
        auto length = stream->GetLength();
        if (length)
            ret->Reserve(*length);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }

    // 从当前位置开始读取所有的内容
    const size_t kChunkSize = 16 * 1024;
    size_t readLength = 0;
    auto& container = ret->GetContainer();
    while (true)
    {
        // 分配 kChunkSize 个数据
        try
        {
            container.resize(readLength + kChunkSize);
        }
        catch (...)
        {
            return make_error_code(errc::not_enough_memory);
        }

        // 读取数据
        auto cnt = stream->Read(container.data() + readLength, kChunkSize);
        if (!cnt)
            return cnt.GetError();

        // 缩小实际大小
        if (*cnt)
        {
            readLength += *cnt;
            container.resize(readLength);
        }
        if (*cnt < kChunkSize)
            break;
    }
    return ret;
}
