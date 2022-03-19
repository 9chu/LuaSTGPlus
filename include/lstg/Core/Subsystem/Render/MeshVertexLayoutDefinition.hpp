/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <tuple>

namespace lstg::Subsystem::Render
{
    /**
     * 顶点布局定义
     */
    class MeshVertexLayoutDefinition
    {
    public:
        /**
         * 字段类型
         */
        enum class ElementScalarTypes: uint8_t
        {
            Int8,
            Int16,
            Int32,
            UInt8,
            UInt16,
            UInt32,
            Half,
            Float,
        };

        /**
         * 字段元素个数
         */
        enum class ElementComponents: uint8_t
        {
            One = 1,
            Two = 2,
            Three = 3,
            Four = 4,
        };

        using ElementType = std::tuple<ElementScalarTypes, ElementComponents>;
    };
}
