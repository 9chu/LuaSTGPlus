/**
 * @file
 * @date 2022/3/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string>
#include "RenderDevice.hpp"
#include "GraphDef/DefinitionError.hpp"
#include "GraphDef/ConstantBufferDefinition.hpp"

namespace Diligent
{
    struct IBuffer;
}

namespace lstg::Subsystem
{
    class RenderSystem;
}

namespace lstg::Subsystem::Render
{
    class Material;
    class EffectFactory;

    /**
     * 常量缓冲区
     * 我们总是在系统内存中保留常量缓冲区的副本，用于提交更改和存取数据。
     */
    class ConstantBuffer
    {
        friend class Material;
        friend class EffectFactory;
        friend class lstg::Subsystem::RenderSystem;

    public:
        enum class Usage
        {
            Default,  ///< @brief 默认
            Dynamic,  ///< @brief 动态，每帧都会失效，使用前需要 Commit
        };

    public:
        ConstantBuffer(RenderDevice& device, GraphDef::ImmutableConstantBufferDefinitionPtr definition, Usage usage);
        ConstantBuffer(const ConstantBuffer&) = delete;
        ConstantBuffer(ConstantBuffer&&) = delete;
        ~ConstantBuffer();

    public:
        /**
         * 获取定义
         */
        [[nodiscard]] const GraphDef::ImmutableConstantBufferDefinitionPtr& GetDefinition() const noexcept;

        /**
         * 获取大小
         */
        [[nodiscard]] size_t GetSize() const noexcept;

        /**
         * 获取变量
         * @tparam T 类型
         * @param name 变量名
         * @return 值
         */
        template <typename T>
        Result<T> GetUniform(std::string_view name) const noexcept
        {
            // 获取字段
            auto field = GetDefinition()->GetField(name);
            if (!field)
                return make_error_code(GraphDef::DefinitionError::SymbolNotFound);

            // 类型检查
            if (!GraphDef::detail::CBufferTypeChecker<T>{}(field->Type))
                return make_error_code(GraphDef::DefinitionError::SymbolTypeMismatched);

            // 获取数据
            assert(field->Offset < m_stBuffer.size() && field->Offset + field->Size <= m_stBuffer.size());
            const uint8_t* memory = m_stBuffer.data() + field->Offset;
            if constexpr (std::is_same_v<std::remove_cv_t<T>, bool>)  // Bool 特殊处理
            {
                int32_t v = 0;
                assert(field->Size == sizeof(v));
                ::memcpy(&v, memory, sizeof(v));
                return static_cast<bool>(v);
            }
            else
            {
                T v = 0;
                assert(field->Size == sizeof(v));
                ::memcpy(&v, memory, sizeof(v));
                return v;
            }
        }

        /**
         * 设置变量
         * @tparam T 类型
         * @param name 变量名
         * @param v 值
         * @return 是否成功
         */
        template <typename T>
        Result<void> SetUniform(std::string_view name, const T& v) noexcept
        {
            // 获取字段
            auto field = GetDefinition()->GetField(name);
            if (!field)
                return make_error_code(GraphDef::DefinitionError::SymbolNotFound);

            // 类型检查
            if (!GraphDef::detail::CBufferTypeChecker<T>{}(field->Type))
                return make_error_code(GraphDef::DefinitionError::SymbolTypeMismatched);

            // 设置数据
            assert(field->Offset < m_stBuffer.size() && field->Offset + field->Size <= m_stBuffer.size());
            if constexpr (std::is_same_v<std::remove_cv_t<T>, bool>)  // Bool 特殊处理
            {
                int32_t val = v ? 1 : 0;
                assert(field->Size == sizeof(val));
                CopyFrom(&val, sizeof(val), field->Offset);
            }
            else
            {
                assert(field->Size == sizeof(v));
                CopyFrom(const_cast<T*>(&v), sizeof(v), field->Offset);
            }
            return {};
        }

    private:
        /**
         * 直接拷贝
         * @param src 源
         * @param size 大小
         * @param offset CBuffer中的偏移
         */
        void CopyFrom(void* src, size_t size, size_t offset) noexcept;

        /**
         * 提交数据
         */
        Result<void> Commit() noexcept;

    private:
        struct DirtyRegion
        {
            size_t Start;
            size_t Size;
        };
        struct DirtyFlag
        {
            bool CommitRequired;
            uint32_t LastCommitFrameId;
        };

        RenderDevice& m_stDevice;
        GraphDef::ImmutableConstantBufferDefinitionPtr m_pDefinition;
        Usage m_iUsage = Usage::Default;
        Diligent::IBuffer* m_pNativeHandler = nullptr;
        std::vector<uint8_t> m_stBuffer;
        union {
            DirtyRegion Region;
            DirtyFlag Flag;
        } m_stDirtyState;
    };

    using ConstantBufferPtr = std::shared_ptr<ConstantBuffer>;
}
