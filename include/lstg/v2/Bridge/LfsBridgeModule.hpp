/**
 * @file
 * @author 9chu
 * @date 2022/9/27
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/VFS/IFileSystem.hpp>
#include <lstg/Core/Subsystem/Script/Unpack.hpp>
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>

namespace lstg::v2::Bridge
{
    /**
     * 文件夹迭代器
     */
    LSTG_CLASS()
    class LfsDirIterator
    {
    public:
        LfsDirIterator(Subsystem::VFS::DirectoryIteratorPtr visitor);

    public:
        /**
         * 返回下一个结果
         */
        LSTG_METHOD(next)
        Subsystem::Script::Unpack<std::optional<std::string>, std::optional<std::string>> Next();

        /**
         * 关闭
         */
        void Close();

    private:
        Subsystem::VFS::DirectoryIteratorPtr m_pVisitor;
    };

    /**
     * LuaFileSystem 兼容层
     */
    LSTG_MODULE(lfs, GLOBAL)
    class LfsBridgeModule
    {
        template <typename... TArgs>
        using Unpack = Subsystem::Script::Unpack<TArgs...>;

        using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;
        using LuaStack = Subsystem::Script::LuaStack;

    public:
        /**
         * stat 操作
         * @param st 栈
         * @param path 路径
         * @param field 属性字段
         */
        LSTG_METHOD(attributes)
        static Unpack<std::variant<std::nullptr_t, AbsIndex>, std::optional<std::string>> GetFileInfo(LuaStack& st, std::string_view path,
            std::optional<std::string_view> field);

        /**
         * 修改当前路径
         * @param st 栈
         * @param path 路径
         */
        LSTG_METHOD(chdir)
        static Unpack<bool, std::optional<std::string>> ChangeDir(LuaStack& st, std::string_view path);

        /**
         * 获取当前路径
         * @param st 栈
         * @return 路径
         */
        LSTG_METHOD(currentdir)
        static std::string_view GetDir(LuaStack& st);

        /**
         * 遍历文件夹
         * @param st 栈
         * @param path 路径
         */
        LSTG_METHOD(dir)
        static Unpack<AbsIndex, LfsDirIterator> IterateDir(LuaStack& st, std::string_view path);

        /**
         * 创建链接
         * @note 总是不支持
         * @param st 栈
         */
        LSTG_METHOD(link)
        static Unpack<std::nullptr_t, std::string_view> MakeLink(LuaStack& st);

        /**
         * 文件锁
         * @note 总是不支持
         * @param st 栈
         */
        LSTG_METHOD(lock)
        static Unpack<std::nullptr_t, std::string_view> FileLock(LuaStack& st);

        /**
         * 创建文件夹
         * @param st 栈
         * @param path 路径
         */
        LSTG_METHOD(mkdir)
        static Unpack<bool, std::optional<std::string>> MakeDir(LuaStack& st, std::string_view path);

        /**
         * 删除文件夹
         * @param st 栈
         * @param path 路径
         */
        LSTG_METHOD(rmdir)
        static Unpack<bool, std::optional<std::string>> RemoveDir(LuaStack& st, std::string_view path);

        /**
         * 获取链接信息
         * @note 总是不支持
         * @param st 栈
         */
        LSTG_METHOD(symlinkattributes)
        static Unpack<std::nullptr_t, std::string_view> GetLinkInfo(LuaStack& st);

        /**
         * 修改文件句柄模式
         * @note 总是不支持
         * @param st 栈
         */
        LSTG_METHOD(setmode)
        static Unpack<std::nullptr_t, std::string_view> SetMode(LuaStack& st);

        /**
         * 修改文件时间
         * @note 总是不支持
         * @param st 栈
         */
        LSTG_METHOD(touch)
        static Unpack<std::nullptr_t, std::string_view> Touch(LuaStack& st);

        /**
         * 解锁文件
         * @note 总是不支持
         * @param st 栈
         */
        LSTG_METHOD(unlock)
        static Unpack<std::nullptr_t, std::string_view> Unlock(LuaStack& st);

        /**
         * 解锁文件夹
         * @note 总是不支持
         * @param st 栈
         */
        LSTG_METHOD(lock_dir)
        static Unpack<std::nullptr_t, std::string_view> LockDir(LuaStack& st);

        /**
         * 删除文件
         * @param st 栈
         * @param path 文件路径
         */
        LSTG_METHOD(rm)
        static Unpack<bool, std::optional<std::string>> RemoveFile(LuaStack& st, std::string_view path);
    };
}
