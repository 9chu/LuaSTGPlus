/**
 * @file
 * @date 2022/3/12
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "ShaderValueType.hpp"
#include "../../../Result.hpp"

namespace lstg::Subsystem::Render::Effect
{
    /**
     * CBuffer 定义
     */
    class ConstantBufferDefinition
    {
    public:
        /**
         * 作用域
         */
        enum class Scope
        {
            Global,
            Local,
        };

    private:
        struct FieldDesc
        {
            std::string Name;  // 字段名称
            ShaderValueType Type;  // 类型
            size_t Size;  // 大小
            size_t Offset;  // 在 CBuffer 中的偏移

            bool operator==(const FieldDesc& rhs) const noexcept
            {
                auto ret = (Name == rhs.Name && Type == rhs.Type && Offset == rhs.Offset);
                assert(!ret || Size == rhs.Size);
                return ret;
            }
        };

    public:
        ConstantBufferDefinition(std::string_view name, Scope scope);

        bool operator==(const ConstantBufferDefinition& rhs) const noexcept;

    public:
        /**
         * 获取 Constant Buffer 的名称
         */
        const std::string& GetName() const noexcept { return m_stName; }

        /**
         * 获取作用域
         */
        Scope GetScope() const noexcept { return m_iScope; }

        /**
         * 计算 Hash 值
         */
        [[nodiscard]] size_t GetHashCode() const noexcept;

        /**
         * 获取对齐
         */
        [[nodiscard]] uint32_t GetAlignment() const noexcept;

        /**
         * 获取大小
         */
        [[nodiscard]] uint32_t GetSize() const noexcept;

        /**
         * 定义字段
         * 字段按照定义顺序在 CBuffer 中声明，自动计算布局大小。
         * @pre !ContainsField(name)
         * @param name 名称
         * @param type 类型
         */
        void DefineField(std::string_view name, ShaderValueType type);

        /**
         * 检查是否存在字段
         * @param name 名称
         */
        [[nodiscard]] bool ContainsField(std::string_view name) const noexcept;

        /**
         * 获取所有字段
         */
        [[nodiscard]] const std::vector<FieldDesc>& GetFields() const noexcept { return m_stFields; }

        /**
         * 转到 HLSL 代码
         */
        void AppendToCode(std::string& out) const;

    private:
        [[nodiscard]] uint32_t CalculateNextFieldOffset(const ShaderValueType& type) const noexcept;

    private:
        std::string m_stName;
        Scope m_iScope;
        std::vector<FieldDesc> m_stFields;
    };

    using ConstantBufferDefinitionPtr = std::shared_ptr<ConstantBufferDefinition>;
    using ImmutableConstantBufferDefinitionPtr = std::shared_ptr<const ConstantBufferDefinition>;
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::Effect::ConstantBufferDefinition>
    {
        std::size_t operator()(const lstg::Subsystem::Render::Effect::ConstantBufferDefinition& value) const
        {
            return value.GetHashCode();
        }
    };
}
