/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "EffectPassGroupDefinition.hpp"

namespace lstg::Subsystem::Render::Effect
{
    /**
     * 效果定义
     */
    class EffectDefinition
    {
    public:
        EffectDefinition() = default;

        bool operator==(const EffectDefinition& rhs) const noexcept;

    public:
        /**
         * 添加渲染组
         * @pre !ContainsGroup(group->GetName())
         * @param group 组
         */
        void AddGroup(ImmutableEffectPassGroupDefinitionPtr group);

        /**
         * 获取渲染组
         */
        [[nodiscard]] const auto& GetGroups() const noexcept { return m_stPassGroups; }

        /**
         * 是否含有指定的组
         * @param name 组名
         */
        [[nodiscard]] bool ContainsGroup(std::string_view name) const noexcept;

        /**
         * 获取哈希值
         */
        [[nodiscard]] size_t GetHashCode() const noexcept;

    private:
        std::vector<ImmutableEffectPassGroupDefinitionPtr> m_stPassGroups;
    };

    using EffectDefinitionPtr = std::shared_ptr<EffectDefinition>;
    using ImmutableEffectDefinitionPtr = std::shared_ptr<const EffectDefinition>;
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::Effect::EffectDefinition>
    {
        std::size_t operator()(const lstg::Subsystem::Render::Effect::EffectDefinition& value) const
        {
            return value.GetHashCode();
        }
    };
}
