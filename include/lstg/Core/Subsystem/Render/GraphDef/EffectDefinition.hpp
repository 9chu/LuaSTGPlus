/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <variant>
#include "EffectPassGroupDefinition.hpp"

namespace lstg::Subsystem::Render::GraphDef
{
    /**
     * 效果定义
     */
    class EffectDefinition
    {
    public:
        /**
         * CBuffer 符号信息
         */
        struct UniformSymbolInfo
        {
            ImmutableConstantBufferDefinitionPtr Definition;
            const ConstantBufferDefinition::FieldDesc* FieldDesc;

            bool operator==(const UniformSymbolInfo& rhs) const noexcept
            {
                return Definition == rhs.Definition && FieldDesc == rhs.FieldDesc;
            }
        };

        /**
         * Texture 符号信息
         */
        struct TextureOrSamplerSymbolInfo
        {
            ImmutableShaderTextureDefinitionPtr Definition;

            bool operator==(const TextureOrSamplerSymbolInfo& rhs) const noexcept
            {
                return Definition == rhs.Definition;
            }
        };

        /**
         * 符号信息
         */
        struct SymbolInfo
        {
            ShaderDefinition::SymbolTypes Type;  // 符号的类型
            std::variant<std::monostate, UniformSymbolInfo, TextureOrSamplerSymbolInfo> AssocInfo;

            bool operator==(const SymbolInfo& rhs) const noexcept
            {
                return Type == rhs.Type && AssocInfo == rhs.AssocInfo;
            }
        };

    public:
        EffectDefinition() = default;

        bool operator==(const EffectDefinition& rhs) const noexcept;

    public:
        /**
         * 添加渲染组
         * @pre !ContainsGroup(group->GetName())
         * @param group 组
         * @warning 当抛出 not_enough_memory 时不保证内部状态一致性
         */
        Result<void> AddGroup(ImmutableEffectPassGroupDefinitionPtr group) noexcept;

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
         * 是否包含指定符号
         * @note 仅查询 Uniform、Texture，不包含 CBuffer
         * @param name 名称
         */
        [[nodiscard]] bool ContainsSymbol(std::string_view name) const noexcept;

        /**
         * 获取符号
         * @note 仅查询 Uniform、Texture，不包含 CBuffer
         * @param symbol 符号名
         */
        [[nodiscard]] const SymbolInfo* GetSymbol(std::string_view symbol) const noexcept;

        /**
         * 获取符号表
         * @note 仅查询 Uniform、Texture，不包含 CBuffer
         */
        [[nodiscard]] const auto& GetSymbolList() const noexcept { return m_stSymbolLookupMap; }

        /**
         * 获取哈希值
         */
        [[nodiscard]] size_t GetHashCode() const noexcept;

    private:
        std::vector<ImmutableEffectPassGroupDefinitionPtr> m_stPassGroups;

        // 查找表
        std::map<std::string, SymbolInfo, std::less<>> m_stSymbolLookupMap;
    };

    using EffectDefinitionPtr = std::shared_ptr<EffectDefinition>;
    using ImmutableEffectDefinitionPtr = std::shared_ptr<const EffectDefinition>;
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::GraphDef::EffectDefinition>
    {
        std::size_t operator()(const lstg::Subsystem::Render::GraphDef::EffectDefinition& value) const noexcept
        {
            return value.GetHashCode();
        }
    };
}
