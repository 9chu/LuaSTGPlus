/**
 * @file
 * @date 2022/2/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <vector>
#include <string>
#include <string_view>

namespace lstg::Subsystem::VFS
{
    namespace detail
    {
        struct SharedPathStorage;
        using SharedPathStoragePtr = std::shared_ptr<const detail::SharedPathStorage>;
    }

    /**
     * 用于表示一个 VFS 路径
     *
     * 一个 VFS 路径具备下述特点：
     *  - 使用 '/' 作为路径分隔符，由若干个文件名构成
     *  - 总是从 '/' 开始
     *  - 文件名不能包含 '/' 或 '\\' 或 '\0'
     *  - '\\' 会被转换成 '/'
     *
     * Path 是不可变对象（immutable），可以安全在多个线程中使用。
     */
    class Path
    {
    public:
        static const char kSeperator = '/';

        /**
         * 规格化路径
         * 去除 '..' '.' 等特殊项目。
         * 去除开头的 '/'。
         * @param path 路径
         * @return 规格化后路径
         */
        static Path Normalize(std::string_view path);

    private:
        struct PathSpan
        {
            detail::SharedPathStoragePtr Storage;
            size_t RangeBegin = 0;
            size_t RangeEnd = 0;  // 开区间
        };

    public:
        Path() = default;
        explicit Path(std::string_view path);
        explicit Path(const std::string& path)
            : Path(std::string_view(path)) {}
        explicit Path(const char* path)
            : Path(std::string_view(path)) {}

    public:
        /**
         * 检查路径是否为空
         */
        operator bool() const noexcept;

        /**
         * 合并路径
         * a / b -> "a/b"
         * a / "/b" -> "/b"
         * @param rhs 右侧的路径
         * @return 合并后的结果
         */
        Path operator/(const Path& rhs) const;

        /**
         * 比较路径是否相同
         * @param rhs 被比较路径
         * @return 是否相同
         */
        bool operator==(const Path& rhs) const noexcept;

        /**
         * 比较路径大小
         * @param rhs 被比较路径
         * @return 是否小于
         */
        bool operator<(const Path& rhs) const noexcept;

        /**
         * 访问器，同 GetSegment
         * @param index 索引
         * @return 路径元素
         */
        std::string_view operator[](size_t index) const noexcept
        {
            return GetSegment(index);
        }

    public:
        /**
         * 当前是否是空路径
         * @note '/' 不是空路径
         */
        bool IsEmpty() const noexcept;

        /**
         * 是否是根目录
         * 检查是否为 '/'
         */
        bool IsRoot() const noexcept;

        /**
         * 检查路径是否是绝对路径
         * 绝对路径总是以 '/' 开头
         */
        bool IsAbsolute() const noexcept;

        /**
         * 获取上级目录
         * e.g.:
         *  "a/b" -> "a"
         *  "a" -> ""
         *  "a/b/c" -> "a/b"
         *  "/a" -> ""
         */
        Path GetParent() const noexcept;

        /**
         * 获取文件名部分
         * e.g.:
         *  "a/b" -> "b"
         *  "a" -> "a"
         *  "/" -> "/"
         */
        Path GetFileName() const noexcept;

        /**
         * 获取多少个构成元素
         */
        size_t GetSegmentCount() const noexcept;

        /**
         * 访问构成元素
         * @param index 索引
         * @return 构成路径的部分
         */
        std::string_view GetSegment(size_t index) const noexcept;

        /**
         * 取切片
         * @param start 起始位置（闭区间）
         * @param end 终止位置（开区间）
         * @return 切片
         */
        Path Slice(size_t start, size_t end=static_cast<size_t>(-1)) const noexcept;

        /**
         * 转换到字符串
         * @note 会产生内存分配
         */
        std::string ToString() const;

        /**
         * 转换到字符串
         */
        std::string_view ToStringView() const noexcept;

    private:
        PathSpan m_stSpan;
    };
}
