/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/GraphDef/ConstantBufferDefinition.hpp>

#include <fmt/format.h>
#include <lstg/Core/Subsystem/Render/GraphDef/DefinitionError.hpp>
#include "detail/NameCheck.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::GraphDef;

namespace
{
    /**
     * 计算对齐后的大小
     * @param size 大小
     * @param alignment 对齐
     * @return 对齐后的大小
     */
    uint32_t AlignedSize(uint32_t size, uint32_t alignment) noexcept
    {
        return (size + alignment - 1) & ~(alignment - 1);
    }
}

ConstantBufferDefinition::ConstantBufferDefinition(std::string_view name)
    : m_stName(name)
{
}

bool ConstantBufferDefinition::operator==(const ConstantBufferDefinition& rhs) const noexcept
{
    auto ret = (m_stName == rhs.m_stName && m_stFields == rhs.m_stFields);
    assert(!ret || m_stFieldsLookupMap == rhs.m_stFieldsLookupMap);
    return ret;
}

size_t ConstantBufferDefinition::GetHashCode() const noexcept
{
    auto ret = std::hash<std::string_view>{}("ConstantBuffer") ^ std::hash<std::string>{}(m_stName);
    for (const auto& f : m_stFields)
    {
        ret ^= std::hash<std::string>{}(f.Name);
        ret ^= f.Type.GetHashCode();
    }
    return ret;
}

uint32_t ConstantBufferDefinition::GetAlignment() const noexcept
{
    return 16;  // 总是 16
}

uint32_t ConstantBufferDefinition::GetSize() const noexcept
{
    if (m_stFields.empty())
        return 0;
    const auto& lastField = m_stFields[m_stFields.size() - 1];
    return AlignedSize(lastField.Size + lastField.Offset, GetAlignment());
}

Result<void> ConstantBufferDefinition::DefineField(std::string_view name, ConstantBufferValueType type) noexcept
{
    if (!detail::IsValidIdentifier(name))
        return make_error_code(DefinitionError::InvalidIdentifier);
    if (name == GetName())
        return make_error_code(DefinitionError::SymbolAlreadyDefined);
    if (ContainsField(name))
        return make_error_code(DefinitionError::SymbolAlreadyDefined);

    auto nextOffset = CalculateNextFieldOffset(type);

    try
    {
        m_stFields.emplace_back(FieldDesc { string{name}, type, type.GetBufferPackingSize(), nextOffset });
        m_stFieldsLookupMap.emplace(name, m_stFields.size() - 1);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }

    assert(m_stFields.size() == m_stFieldsLookupMap.size());
    return {};
}

const ConstantBufferDefinition::FieldDesc* ConstantBufferDefinition::GetField(std::string_view name) const noexcept
{
    auto it = m_stFieldsLookupMap.find(name);
    if (it == m_stFieldsLookupMap.end())
        return nullptr;
    assert(it->second < m_stFields.size());
    return &m_stFields[it->second];
}

bool ConstantBufferDefinition::ContainsField(std::string_view name) const noexcept
{
    if (m_stName == name)
        return true;
    return m_stFieldsLookupMap.find(name) != m_stFieldsLookupMap.end();
}

void ConstantBufferDefinition::AppendToCode(std::string& out) const
{
    fmt::format_to(std::back_inserter(out), "cbuffer {}\n", m_stName);
    out.append("{\n");
    for (const auto& f : m_stFields)
    {
        out.append("    ");
        f.Type.AppendToCode(out);
        fmt::format_to(std::back_inserter(out), " {}; // size:{} offset:{}\n", f.Name, f.Size, f.Offset);
    }
    out.append("}\n");
}

uint32_t ConstantBufferDefinition::CalculateNextFieldOffset(const ConstantBufferValueType& type) const noexcept
{
    auto size = type.GetBufferPackingSize();
    auto align = type.GetBufferPackingAlignment();

    size_t currentTotalSize = 0;
    size_t nextBoundary = 0;
    if (!m_stFields.empty())
    {
        const auto& lastField = m_stFields[m_stFields.size() - 1];
        currentTotalSize = lastField.Offset + lastField.Size;
        nextBoundary = AlignedSize(currentTotalSize, 16);
    }

    // 如果不是 16 字节对齐的，看看剩余空间能不能塞下
    // 16 字节对齐总是塞不下剩余空间的，就不用算了
    if (align < 16)
    {
        // HLSL 要求：如果剩余空间可以塞下对象，则塞下去，否则扩展到下一个 16bytes 边界的地方塞
        auto placeAt = AlignedSize(currentTotalSize, align);
        assert(placeAt % align == 0 && placeAt >= currentTotalSize);
        assert(currentTotalSize + align > placeAt);
        if (placeAt + size <= nextBoundary)  // 塞得下
            return placeAt;
    }

    // 另起一行
    return nextBoundary;
}
