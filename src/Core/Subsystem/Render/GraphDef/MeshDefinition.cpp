/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/GraphDef/MeshDefinition.hpp>

#include <algorithm>
#include <string>
#include <lstg/Core/Subsystem/Render/GraphDef/DefinitionError.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::GraphDef;

bool MeshDefinition::operator==(const MeshDefinition& def) const noexcept
{
    return m_iTopologyType == def.m_iTopologyType &&
        m_iVertexStride == def.m_iVertexStride &&
        m_stVertexElements == def.m_stVertexElements;
}

Result<void> MeshDefinition::AddVertexElement(VertexElementType type, VertexElementSemantic semantic, size_t offset) noexcept
{
    if (ContainsSemantic(semantic))
        return make_error_code(DefinitionError::SymbolAlreadyDefined);

    try
    {
        // 我们按照 Offset 排序
        auto it = std::lower_bound(m_stVertexElements.begin(), m_stVertexElements.end(), offset, [](const VertexElement& e, size_t offset) {
            return e.Offset < offset;
        });

        VertexElement target;
        target.Type = type;
        target.Semantic = semantic;
        target.Offset = offset;

        if (it == m_stVertexElements.end())
        {
            m_stVertexElements.emplace_back(std::move(target));
        }
        else
        {
            // 不允许出现重叠
            assert(it->Offset > offset);
            m_stVertexElements.insert(it, std::move(target));
        }
        return {};
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

bool MeshDefinition::ContainsSemantic(VertexElementSemantic semantic) const noexcept
{
    for (const auto& e : m_stVertexElements)
    {
        if (e.Semantic == semantic)
            return true;
    }
    return false;
}

size_t MeshDefinition::GetHashCode() const noexcept
{
    auto ret = std::hash<string_view>{}("Mesh");
    ret ^= std::hash<PrimitiveTopologyTypes>{}(m_iTopologyType);
    ret ^= std::hash<size_t>{}(m_iVertexStride);
    for (const auto& e : m_stVertexElements)
    {
        ret ^= std::hash<VertexElementScalarTypes>{}(std::get<0>(e.Type));
        ret ^= std::hash<VertexElementComponents>{}(std::get<1>(e.Type));
        ret ^= std::hash<VertexElementSemanticNames>{}(std::get<0>(e.Semantic));
        ret ^= std::hash<uint8_t>{}(std::get<1>(e.Semantic));
        ret ^= std::hash<size_t>{}(e.Offset);
    }
    return ret;
}
