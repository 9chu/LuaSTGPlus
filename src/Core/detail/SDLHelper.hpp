/**
 * @file
 * @author 9chu
 * @date 2022/2/15
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <string>
#include <memory>
#include <system_error>

#include <SDL2/SDL.h>

namespace lstg::Subsystem::VFS
{
    class IStream;
}

namespace lstg::detail
{
    struct SDLDeleter
    {
        template <typename T>
        void operator()(T *p) const
        {
            if (p)
                ::SDL_free(p);
        }
    };

    /**
     * 用于托管SDL分配内存的智能指针
     */
    template <typename T>
    using SDLMemoryPtr = std::unique_ptr<T, SDLDeleter>;

    /**
     * 错误分类声明
     */
    class SDLErrorCategory : public std::error_category
    {
    public:
        static const SDLErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    /**
     * 构造一个 SDL 错误码
     * 事实上大部分情况下 ev 总是等于 -1。
     * @param ev 错误码
     * @return 包含 SDL 错误的错误码
     */
    inline std::error_code MakeSDLError(int ev) noexcept
    {
        return { ev, SDLErrorCategory::GetInstance() };
    }

    /**
     * SDL RWops 析构器
     */
    struct SDLRWOpsDeleter
    {
        void operator()(SDL_RWops* p) noexcept
        {
            ::SDL_RWclose(p);
        }
    };

    using SDLRWOpsPtr = std::unique_ptr<SDL_RWops, SDLRWOpsDeleter>;

    /**
     * 从流创建 RWops
     * 不持有 stream 所有权
     * @param stream 流
     * @return RWops 对象指针，如果内存分配失败，返回 nullptr
     */
    SDLRWOpsPtr CreateRWOpsFromStream(Subsystem::VFS::IStream* stream) noexcept;
}
