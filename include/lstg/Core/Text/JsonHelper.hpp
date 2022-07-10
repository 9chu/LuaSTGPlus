/**
 * @file
 * @date 2022/5/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <optional>
#include <nlohmann/json.hpp>

namespace lstg::Text
{
    /**
     * JSON 辅助类
     */
    class JsonHelper
    {
    public:
        /**
         * 读取值
         * @param object 对象
         * @param path JSON PATH 路径，如 '/foo/bar'
         * @return 如果有值，返回该值，否则返回空
         */
        template <typename T>
        static inline std::optional<T> ReadValue(const nlohmann::json& object, std::string_view path) noexcept
        {
            if (!object.is_object())
                return {};

            try
            {
                auto p = nlohmann::json::json_pointer(std::string{path});
                auto ref = object.at(p);
                return ref.get<T>();
            }
            catch (...)
            {
                return {};
            }
        }

        /**
         * 读取值
         * @tparam T 类型
         * @param object 对象
         * @param path JSON PATH 路径，如 '/foo/bar'
         * @param defaultValue 默认值
         * @return 如果有值，返回该值，否则返回空
         */
        template <typename T>
        static inline T ReadValue(const nlohmann::json& object, std::string_view path, T defaultValue) noexcept
        {
            if (!object.is_object())
                return std::move(defaultValue);

            try
            {
                auto p = nlohmann::json::json_pointer(std::string{path});
                auto ref = object.at(p);
                return ref.get<T>();
            }
            catch (...)
            {
                return std::move(defaultValue);
            }
        }
    };
}
