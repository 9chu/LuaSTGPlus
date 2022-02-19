/**
 * @file
 * @author 9chu
 * @date 2022/2/15
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string>
#include <memory>
#include <system_error>

#include <SDL.h>

namespace lstg
{
    namespace detail
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
    }
}
