/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Effect/ConstantBufferDefinition.hpp>

#include <fmt/format.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect;

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

ConstantBufferDefinition::ConstantBufferDefinition(std::string_view name, Scope scope)
    : m_stName(name), m_iScope(scope)
{
}

bool ConstantBufferDefinition::operator==(const ConstantBufferDefinition& rhs) const noexcept
{
    auto ret = (m_stName == rhs.m_stName && m_iScope == rhs.m_iScope && m_stFields == rhs.m_stFields);
    return ret;
}

size_t ConstantBufferDefinition::GetHashCode() const noexcept
{
    auto ret = std::hash<std::string_view>{}("ConstantBuffer") ^ std::hash<std::string>{}(m_stName) ^
        std::hash<Scope>{}(m_iScope);
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

void ConstantBufferDefinition::DefineField(std::string_view name, ShaderValueType type)
{
    assert(!ContainsField(name));
    auto nextOffset = CalculateNextFieldOffset(type);
    m_stFields.emplace_back(FieldDesc { string{name}, type, type.GetBufferPackingSize(), nextOffset });
}

bool ConstantBufferDefinition::ContainsField(std::string_view name) const noexcept
{
    if (m_stName == name)
        return true;

    for (const auto& f : m_stFields)
    {
        if (f.Name == name)
            return true;
    }

    return false;
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

uint32_t ConstantBufferDefinition::CalculateNextFieldOffset(const ShaderValueType& type) const noexcept
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
