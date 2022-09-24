/**
 * @file
 * @date 2022/5/14
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
                return defaultValue;

            try
            {
                auto p = nlohmann::json::json_pointer(std::string{path});
                if (!object.contains(p))
                    return defaultValue;
                auto ref = object.at(p);
                return ref.get<T>();
            }
            catch (...)
            {
                return defaultValue;
            }
        }
    };
}
