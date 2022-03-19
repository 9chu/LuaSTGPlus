/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Effect/ShaderVertexLayoutDefinition.hpp>

#include <string_view>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect;

bool ShaderVertexLayoutDefinition::operator==(const ShaderVertexLayoutDefinition& rhs) const noexcept
{
    return m_stSlots == rhs.m_stSlots;
}

void ShaderVertexLayoutDefinition::AddSlot(uint32_t index, ElementSemantic semantic, bool normalized)
{
    VertexElementDesc desc;
    desc.SlotIndex = index;
    desc.Semantic = semantic;
    desc.Normalized = normalized;
    auto ret = m_stSlots.emplace(index, std::move(desc));
    assert(ret.second);
    static_cast<void>(ret);
}

bool ShaderVertexLayoutDefinition::ContainsSlot(uint32_t index) const noexcept
{
    return m_stSlots.find(index) != m_stSlots.end();
}

size_t ShaderVertexLayoutDefinition::GetHashCode() const noexcept
{
    auto hash = std::hash<std::string_view>{}("ShaderVertexLayout");
    for (const auto& pair : m_stSlots)
    {
        hash ^= std::hash<uint32_t>{}(pair.second.SlotIndex);
        hash ^= std::hash<ElementSemanticNames>{}(std::get<0>(pair.second.Semantic));
        hash ^= std::hash<uint8_t>{}(std::get<1>(pair.second.Semantic));
        hash ^= std::hash<bool>{}(pair.second.Normalized);
    }
    return hash;
}
