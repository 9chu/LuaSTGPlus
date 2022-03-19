/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <string>
#include <variant>

namespace lstg::Subsystem::Render::Effect
{
    /**
     * Shader 类型
     * 不可变量。
     */
    class ShaderValueType
    {
    public:
        /**
         * 类型
         */
        enum class TypeCategories
        {
            Scalar,  // 标量类型
            Vector,  // 矢量类型
            Matrix,  // 矩阵类型
        };

        /**
         * 标量类型
         */
        enum class ScalarTypes
        {
            Bool,
            Int,
            UInt,
            Float,
            Double,
        };

    public:
        /**
         * 构造一个标量类型
         * @param scalar 标量
         */
        ShaderValueType(ScalarTypes scalar) noexcept;

        /**
         * 构造一个矢量类型
         * @pre 1 <= dimensions && dimensions <= 4
         * @param scalar 标量
         * @param dimensions 矢量维度
         */
        ShaderValueType(ScalarTypes scalar, unsigned dimensions) noexcept;

        /**
         * 构造一个矩阵类型
         * @pre 1 <= row && row <= 4 && 1 <= column && column <= 4
         * @param scalar 标量
         * @param row 行
         * @param column 列
         * @param rowMajor 是否行优先
         */
        ShaderValueType(ScalarTypes scalar, unsigned row, unsigned column, bool rowMajor = false) noexcept;

        bool operator==(const ShaderValueType& rhs) const noexcept;

    public:
        /**
         * 获取分类
         */
        [[nodiscard]] TypeCategories GetCategory() const noexcept;

        /**
         * 获取标量类型
         * 对于标量类型，直接返回。
         * 对于矢量类型，返回标量元素。
         * 对于矩阵类型，返回标量元素。
         */
        [[nodiscard]] ScalarTypes GetScalarType() const noexcept;

        /**
         * 获取矢量的维度
         * @pre GetCategory() == TypeCategories::Vector
         */
        [[nodiscard]] unsigned GetVectorDimensions() const noexcept;

        /**
         * 获取矩阵的行数
         * @pre GetCategory() == TypeCategories::Matrix
         */
        [[nodiscard]] unsigned GetMatrixRows() const noexcept;

        /**
         * 获取矩阵的列数
         * @pre GetCategory() == TypeCategories::Matrix
         */
        [[nodiscard]] unsigned GetMatrixColumns() const noexcept;

        /**
         * 获取矩阵是否是行优先矩阵
         * @pre GetCategory() == TypeCategories::Matrix
         */
        [[nodiscard]] bool IsMatrixRowMajor() const noexcept;

        /**
         * 获取类型大小
         */
        [[nodiscard]] uint32_t GetBufferPackingSize() const noexcept;

        /**
         * 获取对齐字节数
         */
        [[nodiscard]] uint32_t GetBufferPackingAlignment() const noexcept;

        /**
         * 获取 Hash 值
         */
        [[nodiscard]] size_t GetHashCode() const noexcept;

        /**
         * 写到代码
         * @param out 输出缓冲
         */
        void AppendToCode(std::string& out) const;

    private:
        struct Scalar
        {
            ScalarTypes ScalarType;

            bool operator==(const Scalar& rhs) const noexcept
            {
                return ScalarType == rhs.ScalarType;
            }
        };

        struct Vector
        {
            ScalarTypes ScalarType;
            uint32_t Dimensions;

            bool operator==(const Vector& rhs) const noexcept
            {
                return ScalarType == rhs.ScalarType && Dimensions == rhs.Dimensions;
            }
        };

        struct Matrix
        {
            ScalarTypes ScalarType;
            uint32_t Row;
            uint32_t Column;
            bool RowMajor;

            bool operator==(const Matrix& rhs) const noexcept
            {
                return ScalarType == rhs.ScalarType && Row == rhs.Row && Column == rhs.Column && RowMajor == rhs.RowMajor;
            }
        };

        std::variant<Scalar, Vector, Matrix> m_stValue;
    };
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::Effect::ShaderValueType>
    {
        std::size_t operator()(const lstg::Subsystem::Render::Effect::ShaderValueType& value) const
        {
            return value.GetHashCode();
        }
    };
}
