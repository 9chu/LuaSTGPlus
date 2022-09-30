/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/VFS/OverlayFileSystem.hpp>

#include <set>
#include <algorithm>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

namespace
{
    /**
     * 分层文件系统的文件夹迭代器
     *
     * 这里实现较为复杂，需要保证迭代所有层级的文件系统，并保证：
     *   - 迭代过程中能连续产生有效的文件名
     *   - 已经出现过的文件名不能再次出现
     */
    class OverlayFileSystemDirectoryIterator :
        public IDirectoryIterator
    {
    public:
        OverlayFileSystemDirectoryIterator(OverlayFileSystem* fs, Path path)
            : m_pParent(fs), m_stVisitingPath(std::move(path))
        {
            assert(m_pParent);
            ForwardFileSystem();
        }

    public:
        Path GetName() const noexcept override
        {
            if (m_pCurrentIterator)
                return m_pCurrentIterator->GetName();
            return {};
        }

        Result<void> Next() noexcept override
        {
            // 此时迭代已经完成
            if (!m_pCurrentIterator)
                return make_error_code(errc::result_out_of_range);

            while (true)
            {
                // 如果发生错误，直接访问下一个文件系统
                if (!m_pCurrentIterator->Next())
                {
                    auto ec = ForwardFileSystem();
                    if (!ec)
                        return ec.GetError();

                    // ForwardFileSystem 总能保证可以返回一个有效的迭代器
                    assert(m_pCurrentIterator || m_iCurrentFileSystemIndex >= m_pParent->GetFileSystemCount());
                    return {};
                }

                // 否则，检查下是否已经迭代完毕
                if (m_pCurrentIterator->GetName().IsEmpty())
                {
                    // 直接访问下一个文件系统
                    auto ec = ForwardFileSystem();
                    if (!ec)
                        return ec.GetError();

                    // ForwardFileSystem 总能保证可以返回一个有效的迭代器
                    assert(m_pCurrentIterator || m_iCurrentFileSystemIndex >= m_pParent->GetFileSystemCount());
                    return {};
                }

                // 否则，检查是否遍历过这个文件名
                if (m_stVisitedPaths.find(m_pCurrentIterator->GetName()) == m_stVisitedPaths.end())
                {
                    // 如果没有，则添加到已访问列表中
                    m_stVisitedPaths.insert(m_pCurrentIterator->GetName());
                    return {};
                }

                // 此时继续访问下一个文件
            }
        }

    private:
        Result<void> ForwardFileSystem() noexcept
        {
            m_pCurrentIterator.reset();
            while (m_iCurrentFileSystemIndex < m_pParent->GetFileSystemCount())
            {
                auto index = m_iCurrentFileSystemIndex++;
                auto layer = m_pParent->GetFileSystem(m_pParent->GetFileSystemCount() - (index + 1));
                auto it = layer->VisitDirectory(m_stVisitingPath);
                if (it)
                {
                    m_pCurrentIterator = std::move(*it);

                    // 设置迭代器后，我们要保证这个迭代器能拿到一个有效的文件名

                    // 如果为空，则跳过并继续迭代
                    if (m_pCurrentIterator->GetName().IsEmpty())
                    {
                        m_pCurrentIterator.reset();
                        goto CONT;
                    }

                    // 如果已经出现同名文件，跳过，保证迭代只返回一个同名文件
                    while (m_stVisitedPaths.find(m_pCurrentIterator->GetName()) != m_stVisitedPaths.end())
                    {
                        auto ret = m_pCurrentIterator->Next();

                        // 迭代失败，跳过当前迭代器
                        if (!ret)
                        {
                            m_pCurrentIterator.reset();
                            goto CONT;
                        }

                        // 如果为空，则跳过并继续迭代
                        if (m_pCurrentIterator->GetName().IsEmpty())
                        {
                            m_pCurrentIterator.reset();
                            goto CONT;
                        }
                    }

                    // 此时找到一个有效的文件
                    assert(m_pCurrentIterator && !m_pCurrentIterator->GetName().IsEmpty() &&
                        m_stVisitedPaths.find(m_pCurrentIterator->GetName()) == m_stVisitedPaths.end());

                    // 加入集合
                    try
                    {
                        m_stVisitedPaths.insert(m_pCurrentIterator->GetName());
                    }
                    catch (...)
                    {
                        return make_error_code(errc::not_enough_memory);
                    }
                    return {};
                }

            CONT:;
            }
            assert(!m_pCurrentIterator);
            return {};
        }

    private:
        OverlayFileSystem* m_pParent = nullptr;
        Path m_stVisitingPath;
        size_t m_iCurrentFileSystemIndex = 0;
        DirectoryIteratorPtr m_pCurrentIterator;
        std::set<Path> m_stVisitedPaths;
    };
}

void OverlayFileSystem::PushFileSystem(FileSystemPtr fs)
{
    m_stFileSystems.emplace_back(std::move(fs));
}

bool OverlayFileSystem::PopFileSystem() noexcept
{
    if (m_stFileSystems.empty())
        return false;
    m_stFileSystems.pop_back();
    return true;
}

bool OverlayFileSystem::RemoveFileSystem(const FileSystemPtr& fs) noexcept
{
    auto it = std::find(m_stFileSystems.begin(), m_stFileSystems.end(), fs);
    if (it != m_stFileSystems.end())
    {
        m_stFileSystems.erase(it);
        return true;
    }
    return false;
}

void OverlayFileSystem::RemoveFileSystemAt(size_t index) noexcept
{
    assert(index < m_stFileSystems.size());
    m_stFileSystems.erase(m_stFileSystems.begin() + index);
}

size_t OverlayFileSystem::GetFileSystemCount() const noexcept
{
    return m_stFileSystems.size();
}

FileSystemPtr OverlayFileSystem::GetFileSystem(size_t index) const noexcept
{
    assert(index < m_stFileSystems.size());
    return m_stFileSystems[index];
}

Result<void> OverlayFileSystem::CreateDirectory(Path path) noexcept
{
    auto ec = make_error_code(errc::not_supported);
    for (auto it = m_stFileSystems.rbegin(); it != m_stFileSystems.rend(); ++it)
    {
        auto ret = (*it)->CreateDirectory(path);
        if (ret)
            return {};
        if (ret.GetError() != make_error_code(errc::not_supported))
            ec = ret.GetError();
    }
    return ec;
}

Result<void> OverlayFileSystem::Remove(Path path) noexcept
{
    auto ec = make_error_code(errc::no_such_file_or_directory);
    for (auto it = m_stFileSystems.rbegin(); it != m_stFileSystems.rend(); ++it)
    {
        auto ret = (*it)->Remove(path);
        if (ret)
            return {};
        if (ret.GetError() != make_error_code(errc::not_supported) &&
            ret.GetError() != make_error_code(errc::no_such_file_or_directory))
        {
            ec = ret.GetError();
        }
    }
    return ec;
}

Result<void> OverlayFileSystem::Rename(Path from, Path to) noexcept
{
    auto ec = make_error_code(errc::no_such_file_or_directory);
    for (auto it = m_stFileSystems.rbegin(); it != m_stFileSystems.rend(); ++it)
    {
        auto ret = (*it)->Rename(from, to);
        if (ret)
            return {};
        if (ret.GetError() != make_error_code(errc::not_supported) &&
            ret.GetError() != make_error_code(errc::no_such_file_or_directory))
        {
            ec = ret.GetError();
        }
    }
    return ec;
}

Result<FileAttribute> OverlayFileSystem::GetFileAttribute(Path path) noexcept
{
    auto ec = make_error_code(errc::no_such_file_or_directory);
    for (auto it = m_stFileSystems.rbegin(); it != m_stFileSystems.rend(); ++it)
    {
        auto ret = (*it)->GetFileAttribute(path);
        if (ret)
            return ret;
        if (ret.GetError() != make_error_code(errc::not_supported) &&
            ret.GetError() != make_error_code(errc::no_such_file_or_directory))
        {
            ec = ret.GetError();
        }
    }
    return ec;
}

Result<DirectoryIteratorPtr> OverlayFileSystem::VisitDirectory(Path path) noexcept
{
    try
    {
        return make_shared<OverlayFileSystemDirectoryIterator>(this, std::move(path));
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (const bad_alloc& ex)
    {
        return make_error_code(errc::not_enough_memory);
    }
    catch (...)
    {
        return make_error_code(errc::io_error);
    }
}

Result<StreamPtr> OverlayFileSystem::OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept
{
    auto ec = make_error_code(errc::no_such_file_or_directory);
    for (auto it = m_stFileSystems.rbegin(); it != m_stFileSystems.rend(); ++it)
    {
        auto ret = (*it)->OpenFile(path, access, flags);
        if (ret)
            return ret;
        if (ret.GetError() != make_error_code(errc::not_supported) &&
            ret.GetError() != make_error_code(errc::no_such_file_or_directory))
        {
            ec = ret.GetError();
        }
    }
    return ec;
}

const std::string& OverlayFileSystem::GetUserData() const noexcept
{
    return m_stUserData;
}

void OverlayFileSystem::SetUserData(std::string ud) noexcept
{
    m_stUserData = std::move(ud);
}
