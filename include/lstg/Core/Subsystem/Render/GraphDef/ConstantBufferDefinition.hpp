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
#include "ConstantBufferValueType.hpp"
#include "../../../Result.hpp"

namespace lstg::Subsystem::Render::GraphDef
{
    /**
     * CBuffer 定义
     */
    class ConstantBufferDefinition
    {
    public:
        struct FieldDesc
        {
            std::string Name;  // 字段名称
            ConstantBufferValueType Type;  // 类型
            size_t Size = 0;  // 大小
            size_t Offset = 0;  // 在 CBuffer 中的偏移

            bool operator==(const FieldDesc& rhs) const noexcept
            {
                auto ret = (Name == rhs.Name && Type == rhs.Type && Offset == rhs.Offset);
                assert(!ret || Size == rhs.Size);
                return ret;
            }
        };

    public:
        ConstantBufferDefinition(std::string_view name);

        bool operator==(const ConstantBufferDefinition& rhs) const noexcept;

    public:
        /**
         * 获取 Constant Buffer 的名称
         */
        [[nodiscard]] const std::string& GetName() const noexcept { return m_stName; }

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
         * @param name 名称
         * @param type 类型
         * @return 是否成功，如果字段已经定义，返回失败
         * @warning 当抛出 not_enough_memory 时不保证内部状态一致性
         */
        Result<void> DefineField(std::string_view name, ConstantBufferValueType type) noexcept;

        /**
         * 获取字段描述
         * @param name 字段名
         * @return 描述对象
         */
        [[nodiscard]] const FieldDesc* GetField(std::string_view name) const noexcept;

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
        [[nodiscard]] uint32_t CalculateNextFieldOffset(const ConstantBufferValueType& type) const noexcept;

    private:
        std::string m_stName;
        std::vector<FieldDesc> m_stFields;

        // 查找表
        std::map<std::string, size_t, std::less<>> m_stFieldsLookupMap;
    };

    using ConstantBufferDefinitionPtr = std::shared_ptr<ConstantBufferDefinition>;
    using ImmutableConstantBufferDefinitionPtr = std::shared_ptr<const ConstantBufferDefinition>;
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::GraphDef::ConstantBufferDefinition>
    {
        std::size_t operator()(const lstg::Subsystem::Render::GraphDef::ConstantBufferDefinition& value) const noexcept
        {
            return value.GetHashCode();
        }
    };
}
