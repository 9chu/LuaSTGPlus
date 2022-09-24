/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/GraphDef/ConstantBufferValueType.hpp>

#include <fmt/format.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::GraphDef;

namespace
{
    /**
     * 获取标量类型大小
     * @param scalar 标量类型
     */
    uint32_t GetScalarSize(ConstantBufferValueType::ScalarTypes scalar) noexcept
    {
        // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-scalar
        switch (scalar)
        {
            case ConstantBufferValueType::ScalarTypes::Bool:
            case ConstantBufferValueType::ScalarTypes::Int:
            case ConstantBufferValueType::ScalarTypes::UInt:
            case ConstantBufferValueType::ScalarTypes::Float:
                return 4;
            case ConstantBufferValueType::ScalarTypes::Double:
                return 8;
            default:
                assert(false);
                return 0;
        }
    }

    /**
     * 获取标量类型的对齐
     * @param scalar 标量类型
     */
    uint32_t GetScalarAlignment(ConstantBufferValueType::ScalarTypes scalar) noexcept
    {
        // 标量类型的对齐和本身的大小一致
        return GetScalarSize(scalar);
    }

    /**
     * 写到代码
     * @param out 代码输出
     * @param scalar 标量
     */
    void AppendToCode(std::string& out, ConstantBufferValueType::ScalarTypes scalar)
    {
        switch (scalar)
        {
            case ConstantBufferValueType::ScalarTypes::Bool:
                out.append("bool");
                break;
            case ConstantBufferValueType::ScalarTypes::Int:
                out.append("int");
                break;
            case ConstantBufferValueType::ScalarTypes::UInt:
                out.append("uint");
                break;
            case ConstantBufferValueType::ScalarTypes::Float:
                out.append("float");
                break;
            case ConstantBufferValueType::ScalarTypes::Double:
                out.append("double");
                break;
            default:
                assert(false);
                break;
        }
    }
}

ConstantBufferValueType::ConstantBufferValueType(ScalarTypes scalar) noexcept
    : m_stValue(Scalar { scalar })
{
}

ConstantBufferValueType::ConstantBufferValueType(ScalarTypes scalar, unsigned dimensions) noexcept
    : m_stValue(Vector { scalar, dimensions })
{
    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-vector
    assert(1 <= dimensions && dimensions <= 4);
}

ConstantBufferValueType::ConstantBufferValueType(ScalarTypes scalar, unsigned row, unsigned column, bool rowMajor) noexcept
    : m_stValue(Matrix { scalar, row, column, rowMajor })
{
    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-matrix
    assert(1 <= row && row <= 4 && 1 <= column && column <= 4);
}

bool ConstantBufferValueType::operator==(const ConstantBufferValueType& rhs) const noexcept
{
    return m_stValue == rhs.m_stValue;
}

ConstantBufferValueType::TypeCategories ConstantBufferValueType::GetCategory() const noexcept
{
    switch (m_stValue.index())
    {
        case 0:
            return TypeCategories::Scalar;
        case 1:
            return TypeCategories::Vector;
        case 2:
            return TypeCategories::Matrix;
        default:
            assert(false);
            return TypeCategories::Scalar;
    }
}

ConstantBufferValueType::ScalarTypes ConstantBufferValueType::GetScalarType() const noexcept
{
    switch (m_stValue.index())
    {
        case 0:
            return std::get<0>(m_stValue).ScalarType;
        case 1:
            return std::get<1>(m_stValue).ScalarType;
        case 2:
            return std::get<2>(m_stValue).ScalarType;
        default:
            assert(false);
            return ConstantBufferValueType::ScalarTypes::Float;
    }
}

unsigned ConstantBufferValueType::GetVectorDimensions() const noexcept
{
    assert(m_stValue.index() == 1);
    return std::get<1>(m_stValue).Dimensions;
}

unsigned ConstantBufferValueType::GetMatrixRows() const noexcept
{
    assert(m_stValue.index() == 2);
    return std::get<2>(m_stValue).Row;
}

unsigned ConstantBufferValueType::GetMatrixColumns() const noexcept
{
    assert(m_stValue.index() == 2);
    return std::get<2>(m_stValue).Column;
}

bool ConstantBufferValueType::IsMatrixRowMajor() const noexcept
{
    assert(m_stValue.index() == 2);
    return std::get<2>(m_stValue).RowMajor;
}

uint32_t ConstantBufferValueType::GetBufferPackingSize() const noexcept
{
    // https://github.com/microsoft/DirectXShaderCompiler/wiki/Buffer-Packing
    switch (m_stValue.index())
    {
        case 0:  // 标量类型
            return GetScalarSize(std::get<0>(m_stValue).ScalarType);
        case 1:  // 矢量类型
            return GetScalarSize(std::get<1>(m_stValue).ScalarType) * std::get<1>(m_stValue).Dimensions;
        case 2:  // 矩阵类型
            {
                const auto& matrix = std::get<2>(m_stValue);

                // 这两种情况直接当 Vector 对待
                if (matrix.RowMajor && matrix.Row == 1)
                    return GetScalarSize(matrix.ScalarType) * matrix.Column;
                if (!matrix.RowMajor && matrix.Column == 1)
                    return GetScalarSize(matrix.ScalarType) * matrix.Row;

                // 我们按照 row_major 计算
                auto row = matrix.Row;
                auto column = matrix.Column;
                if (!matrix.RowMajor)
                    std::swap(row, column);

                // 计算一行的大小
                auto rowSize = GetScalarSize(matrix.ScalarType) * column;

                // 按照 16 字节对齐
                // auto rowAlignmentSize = static_cast<unsigned>(::ceil(static_cast<double>(rowSize) / 16.) * 16.);
                auto rowAlignmentSize = (rowSize + 16 - 1) & ~(16 - 1);
                assert((rowAlignmentSize % 16 == 0) && rowSize <= rowAlignmentSize && rowSize + 16 > rowAlignmentSize);

                return rowAlignmentSize * row;
            }
        default:
            assert(false);
            return 0;
    }
}

uint32_t ConstantBufferValueType::GetBufferPackingAlignment() const noexcept
{
    // https://github.com/microsoft/DirectXShaderCompiler/wiki/Buffer-Packing
    switch (m_stValue.index())
    {
        case 0:  // 标量类型
            return GetScalarAlignment(std::get<0>(m_stValue).ScalarType);
        case 1:  // 矢量类型
            // 矢量类型的对齐和元素对齐一致
            return GetScalarAlignment(std::get<1>(m_stValue).ScalarType);
        case 2:  // 矩阵类型
            {
                const auto& matrix = std::get<2>(m_stValue);

                // 这两种情况直接当 Vector 对待
                if ((matrix.RowMajor && matrix.Row == 1) || (!matrix.RowMajor && matrix.Column == 1))
                    return GetScalarAlignment(matrix.ScalarType);

                // 不是一行的，直接按照 16 字节对齐
                return 16;
            }
        default:
            assert(false);
            return 0;
    }
}

size_t ConstantBufferValueType::GetHashCode() const noexcept
{
    auto hash = std::hash<std::string_view>{}("ShaderType");

    switch (m_stValue.index())
    {
        case 0:
            hash ^= std::hash<uint32_t>{}((static_cast<uint32_t>(std::get<0>(m_stValue).ScalarType) << 24) | 0x1);
            break;
        case 1:
            hash ^= std::hash<uint32_t>{}((static_cast<uint32_t>(std::get<1>(m_stValue).ScalarType) << 24) |
                (static_cast<uint32_t>(std::get<1>(m_stValue).Dimensions) << 16) |
                0x2);
            break;
        case 2:
            hash ^= std::hash<uint32_t>{}((static_cast<uint32_t>(std::get<2>(m_stValue).ScalarType) << 24) |
                (static_cast<uint32_t>(std::get<2>(m_stValue).Row) << 16) |
                (static_cast<uint32_t>(std::get<2>(m_stValue).Column) << 8) |
                0x3);
            break;
        default:
            assert(false);
            break;
    }
    return hash;
}

void ConstantBufferValueType::AppendToCode(std::string& out) const
{
    switch (m_stValue.index())
    {
        case 0:
            ::AppendToCode(out, std::get<0>(m_stValue).ScalarType);
            break;
        case 1:
            {
                const auto& vector = std::get<1>(m_stValue);

                ::AppendToCode(out, vector.ScalarType);
                fmt::format_to(std::back_inserter(out), "{}", vector.Dimensions);
            }
            break;
        case 2:
            {
                const auto& matrix = std::get<2>(m_stValue);

                if (matrix.RowMajor)
                    out.append("row_major ");
                ::AppendToCode(out, matrix.ScalarType);
                fmt::format_to(std::back_inserter(out), "{}x{}", matrix.Row, matrix.Column);
            }
            break;
        default:
            assert(false);
            break;
    }
}
