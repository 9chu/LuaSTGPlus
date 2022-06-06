/**
 * @file
 * @date 2022/2/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/VFS/Path.hpp>

#include <cassert>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

namespace lstg::Subsystem::VFS::detail
{
    struct SharedPathStorage
    {
        std::vector<std::string_view> FileNames;
        size_t FullPathLength = 0;
        char FullPath[1];
    };

    struct SharedPathStorageDeleter
    {
        void operator()(SharedPathStorage* p) noexcept
        {
            if (p)
            {
                p->~SharedPathStorage();
                ::free(p);
            }
        }
    };
}

namespace
{
    /**
     * 从文件名创建共享的路径存储数据结构
     * 并且对路径进行规格化，消除空路径。
     * @param path 路径
     * @return 共享存储
     */
    detail::SharedPathStoragePtr CreatePathStorageFromString(std::string_view path)
    {
        using detail::SharedPathStorage;
        using detail::SharedPathStoragePtr;

        enum {
            STATE_CHECK_IF_ABSOLUTELY,
            STATE_CHECK_IF_EMPTY,
            STATE_WAIT_SEPERATOR,
        };

        string buffer;
        vector<tuple<size_t, size_t>> spans;
        int state = STATE_CHECK_IF_ABSOLUTELY;
        size_t curSpanBegin = 0;
        size_t curSpanEnd = 0;

        // 规格化后不可能比原来还长
        // FIXME: 或许我们可以直接申请一个 SharedPathStorage
        buffer.reserve(path.length());

        for (size_t i = 0; i <= path.length(); ++i)
        {
            auto ch = (i == path.length()) ? '\0' : path[i];

            // 为了保证在 Windows/UNIX 环境兼容，这里会把 '\\' 当做 '/' 处理
            // 换言之我们不允许在 Path 中出现 '\\'
            if (ch == '\\')
                ch = '/';

            switch (state)
            {
                case STATE_CHECK_IF_ABSOLUTELY:
                    if (ch == '/')
                    {
                        buffer.push_back('/');
                        state = STATE_CHECK_IF_EMPTY;
                        break;
                    }
                    state = STATE_CHECK_IF_EMPTY;
                    [[fallthrough]];
                case STATE_CHECK_IF_EMPTY:
                    if (ch == '/' || ch == '\0')
                        break;
                    state = STATE_WAIT_SEPERATOR;
                    curSpanBegin = buffer.length();
                    curSpanEnd = 0;
                    buffer.push_back(ch);
                    break;
                case STATE_WAIT_SEPERATOR:
                    if (ch == '/' || ch == '\0')
                    {
                        curSpanEnd = buffer.length();
                        assert(curSpanBegin < curSpanEnd);
                        spans.emplace_back<tuple<size_t, size_t>>({curSpanBegin, curSpanEnd - curSpanBegin});

                        // 如果后面还有数据，说明这不是最后一个路径构成元素，我们加入一个分隔符
                        if (ch != '\0')
                            buffer.push_back('/');

                        state = STATE_CHECK_IF_EMPTY;
                        break;
                    }
                    buffer.push_back(ch);
                    break;
                default:
                    assert(false);
                    break;
            }

            // 如果在路径中间有 '\0'，忽略后续的字符
            if (ch == '\0')
                break;
        }
        assert(state == STATE_CHECK_IF_EMPTY);

        // 如果没有内容，直接返回空指针
        if (buffer.empty())
            return nullptr;
        assert(buffer == "/" || !spans.empty());

        // 根据结果构成 PathStorage
        auto storage = static_cast<SharedPathStorage*>(::malloc(sizeof(detail::SharedPathStorage) + buffer.length()));
        if (!storage)
            throw std::bad_alloc();
        new(storage) SharedPathStorage();

        // 转移到 shared_ptr 中
        std::shared_ptr<SharedPathStorage> ret;
        ret.reset(storage, detail::SharedPathStorageDeleter{});

        // 复制结果
        ret->FullPathLength = buffer.length();
        ::memcpy(ret->FullPath, buffer.data(), buffer.length());
        ret->FullPath[buffer.length()] = '\0';  // detail::SharedPathStorage 里面多一个字节，刚好用来放 '\0'
        ret->FileNames.reserve(spans.size());

        // span 转绝对地址
        for (const auto& span : spans)
            ret->FileNames.emplace_back(ret->FullPath + std::get<0>(span), std::get<1>(span));

        return static_pointer_cast<const SharedPathStorage>(ret);
    }
}

Path Path::Normalize(std::string_view path)
{
    // FIXME: 优化效率

    VFS::Path temp(path);

    // 过滤 '.' 和 '..'
    vector<string> normalized;
    normalized.reserve(temp.GetSegmentCount());
    for (size_t i = 0; i < temp.GetSegmentCount(); ++i)
    {
        auto segment = temp[i];
        assert(!segment.empty());

        if (segment == ".")
            continue;
        if (segment == "..")
        {
            if (!normalized.empty())
                normalized.pop_back();
            continue;
        }
        normalized.emplace_back(segment);
    }

    // 合并路径
    string join;
    join.reserve(path.length());
    for (auto& e : normalized)
    {
        if (!join.empty())
            join.append("/");
        join.append(e);
    }

    return VFS::Path {join};
}

Path::Path(std::string_view path)
{
    auto storage = CreatePathStorageFromString(path);
    if (storage)
    {
        m_stSpan.Storage = storage;
        m_stSpan.RangeBegin = 0;
        m_stSpan.RangeEnd = storage->FileNames.size();
    }
}

Path::operator bool() const noexcept
{
    return IsEmpty();
}

Path Path::operator/(const Path& rhs) const
{
    using detail::SharedPathStorage;
    using detail::SharedPathStorageDeleter;

    // fast path
    if (IsEmpty() || rhs.IsAbsolute())
        return rhs;
    if (rhs.IsEmpty())
        return *this;

    // 计算合并后路径长度和路径构成元素个数
    auto left = ToStringView();
    auto right = ToStringView();
    assert(!right.empty() && right[0] != '/');

    auto pathLength = left.length() + right.length() + 1;  // 分隔符空间另算
    auto spanCount = (m_stSpan.RangeEnd - m_stSpan.RangeBegin) + (rhs.m_stSpan.RangeEnd - rhs.m_stSpan.RangeBegin);

    // 分配 PathStorage
    auto storage = static_cast<SharedPathStorage*>(::malloc(sizeof(SharedPathStorage) + pathLength));
    if (!storage)
        throw std::bad_alloc();
    new(storage) SharedPathStorage();

    // 转移到 shared_ptr 中
    std::shared_ptr<SharedPathStorage> ret;
    ret.reset(storage, SharedPathStorageDeleter{});

    // 复制结果
    ret->FullPathLength = pathLength;
    ::memcpy(ret->FullPath, left.data(), left.length());
    ret->FullPath[left.length()] = '/';
    ::memcpy(ret->FullPath + left.length() + 1, right.data(), right.length());
    ret->FullPath[pathLength] = '\0';  // detail::SharedPathStorage 里面多一个字节，刚好用来放 '\0'
    ret->FileNames.reserve(spanCount);

    // 复制左侧的 span
    for (size_t i = m_stSpan.RangeBegin; i < m_stSpan.RangeEnd; ++i)
    {
        assert(i < m_stSpan.Storage->FileNames.size());
        auto s = m_stSpan.Storage->FileNames[i];
        assert(s.data() >= left.data());
        ret->FileNames.emplace_back(s.data() - left.data() + ret->FullPath, s.length());
    }

    // 复制右侧的 span
    auto base = ret->FullPath + left.length() + 1;
    for (size_t i = rhs.m_stSpan.RangeBegin; i < rhs.m_stSpan.RangeEnd; ++i)
    {
        assert(i < rhs.m_stSpan.Storage->FileNames.size());
        auto s = rhs.m_stSpan.Storage->FileNames[i];
        assert(s.data() >= right.data());
        ret->FileNames.emplace_back(s.data() - right.data() + base, s.length());
    }

    // 产生返回结果
    Path p;
    p.m_stSpan.Storage = ret;
    p.m_stSpan.RangeBegin = 0;
    p.m_stSpan.RangeEnd = ret->FileNames.size();
    return p;
}

bool Path::operator==(const Path& rhs) const noexcept
{
    auto left = ToStringView();
    auto right = rhs.ToStringView();
    return left == right;
}

bool Path::operator<(const Path& rhs) const noexcept
{
    auto left = ToStringView();
    auto right = rhs.ToStringView();
    return left < right;
}

bool Path::IsEmpty() const noexcept
{
    return !m_stSpan.Storage;
}

bool Path::IsRoot() const noexcept
{
    return m_stSpan.Storage && m_stSpan.Storage->FullPathLength == 1 && m_stSpan.Storage->FullPath[0] == '/';
}

bool Path::IsAbsolute() const noexcept
{
    if (m_stSpan.Storage)
    {
        assert(m_stSpan.Storage->FullPathLength > 0);
        return m_stSpan.Storage->FullPath[0] == '/';
    }
    return false;
}

Path Path::GetParent() const noexcept
{
    if (!m_stSpan.Storage)
        return {};
    if (m_stSpan.RangeBegin + 1 >= m_stSpan.RangeEnd)  // 保证至少有两个元素
        return {};
    assert(m_stSpan.RangeEnd - m_stSpan.RangeBegin >= 2);

    Path ret = *this;
    ret.m_stSpan.RangeEnd -= 1;
    return ret;
}

Path Path::GetFileName() const noexcept
{
    if (!m_stSpan.Storage)
        return {};
    if (IsRoot())
        return *this;
    assert(m_stSpan.RangeEnd > m_stSpan.RangeBegin);

    Path ret = *this;
    ret.m_stSpan.RangeBegin = ret.m_stSpan.RangeEnd - 1;
    return ret;
}

size_t Path::GetSegmentCount() const noexcept
{
    return m_stSpan.RangeEnd - m_stSpan.RangeBegin;
}

std::string_view Path::GetSegment(size_t index) const noexcept
{
    if (!m_stSpan.Storage || index >= GetSegmentCount())
        return {};
    return m_stSpan.Storage->FileNames[m_stSpan.RangeBegin + index];
}

Path Path::Slice(size_t start, size_t end) const noexcept
{
    if (!m_stSpan.Storage)
        return {};
    if (start >= GetSegmentCount())
        return {};
    if (end > GetSegmentCount())
        end = GetSegmentCount();
    if (start >= end)
        return {};

    Path ret = *this;
    ret.m_stSpan.RangeEnd = ret.m_stSpan.RangeBegin + end;
    ret.m_stSpan.RangeBegin += start;
    return ret;
}

std::string Path::ToString() const
{
    auto v = ToStringView();
    if (v.empty())
        return {};
    return {v.data(), v.size()};
}

std::string_view Path::ToStringView() const noexcept
{
    if (IsEmpty())
        return {};
    if (IsRoot())
        return m_stSpan.Storage->FullPath;
    assert(m_stSpan.RangeEnd > m_stSpan.RangeBegin);

    // 当 RangeBegin 是 0 时，包含首个 '/'（若有）
    auto begin = m_stSpan.RangeBegin == 0 ? m_stSpan.Storage->FullPath :
        m_stSpan.Storage->FileNames[m_stSpan.RangeBegin].data();
    auto lastView = m_stSpan.Storage->FileNames[m_stSpan.RangeEnd - 1];
    auto end = lastView.data() + lastView.size();
    assert(end > begin);
    return {begin, static_cast<size_t>(end - begin)};
}
