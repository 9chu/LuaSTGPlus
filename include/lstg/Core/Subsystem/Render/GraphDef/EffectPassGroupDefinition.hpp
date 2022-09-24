/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <string>
#include <string_view>
#include "EffectPassDefinition.hpp"

namespace lstg::Subsystem::Render::GraphDef
{
    /**
     * 渲染过程组定义
     */
    class EffectPassGroupDefinition
    {
    public:
        EffectPassGroupDefinition() = default;

        bool operator==(const EffectPassGroupDefinition& rhs) const noexcept;

    public:
        /**
         * 获取 Pass 组名称
         */
        [[nodiscard]] const std::string& GetName() const noexcept { return m_stName; }

        /**
         * 设置 Pass 组名称
         */
        void SetName(std::string name) noexcept { m_stName = std::move(name); }

        /**
         * 获取标签
         * @param key 键
         * @return 标签值，若不存在则返回空串
         */
        [[nodiscard]] std::string_view GetTag(std::string_view key) const noexcept;

        /**
         * 设置标签
         * @param key 键
         * @param value 值
         */
        void SetTag(std::string_view key, std::string_view value);

        /**
         * 增加一个 Pass 定义
         * @pre !ContainsPass(pass)
         * @param pass Pass 对象
         */
        Result<void> AddPass(ImmutableEffectPassDefinitionPtr pass) noexcept;

        /**
         * 检查指定名字的 Pass 是否存在
         * @param name 名称
         */
        [[nodiscard]] bool ContainsPass(std::string_view name) const noexcept;

        /**
         * 获取所有 Pass
         */
        [[nodiscard]] const auto& GetPasses() const noexcept { return m_stPasses; }

        /**
         * 计算哈希值
         */
        [[nodiscard]] size_t GetHashCode() const noexcept;

    private:
        std::string m_stName;
        std::map<std::string, std::string, std::less<>> m_stTags;
        std::vector<ImmutableEffectPassDefinitionPtr> m_stPasses;
    };

    using EffectPassGroupDefinitionPtr = std::shared_ptr<const EffectPassGroupDefinition>;
    using ImmutableEffectPassGroupDefinitionPtr = std::shared_ptr<const EffectPassGroupDefinition>;
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::GraphDef::EffectPassGroupDefinition>
    {
        std::size_t operator()(const lstg::Subsystem::Render::GraphDef::EffectPassGroupDefinition& value) const noexcept
        {
            return value.GetHashCode();
        }
    };
}
