/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string>
#include <string_view>
#include <set>
#include "ConstantBufferDefinition.hpp"
#include "ShaderTextureDefinition.hpp"
#include "ShaderVertexLayoutDefinition.hpp"

namespace lstg::Subsystem::Render::Effect
{
    /**
     * Shader 定义
     */
    class ShaderDefinition
    {
    public:
        enum class ShaderTypes
        {
            VertexShader,
            PixelShader,
        };

    public:
        ShaderDefinition(ShaderTypes type);

        bool operator==(const ShaderDefinition& rhs) const noexcept;

    public:
        /**
         * 获取 Shader 类型
         */
        [[nodiscard]] ShaderTypes GetType() const noexcept { return m_iType; }

        /**
         * 获取名称
         */
        [[nodiscard]] const std::string& GetName() const noexcept { return m_stName; }

        /**
         * 设置名称
         * @param name 名称
         */
        void SetName(std::string name) noexcept { m_stName = std::move(name); }

        /**
         * 获取源码
         */
        [[nodiscard]] const std::string& GetSource() const noexcept { return m_stSource; }

        /**
         * 设置源码
         * @param source 源码
         */
        void SetSource(std::string source) noexcept { m_stSource = std::move(source); }

        /**
         * 获取入口
         */
        [[nodiscard]] const std::string& GetEntry() const noexcept { return m_stEntry; }

        /**
         * 设置入口
         * @param entry 入口
         */
        void SetEntry(std::string entry) noexcept { m_stEntry = std::move(entry); }

        /**
         * 获取 CBuffer 引用列表
         */
        [[nodiscard]] const auto& GetConstantBufferReferences() const noexcept { return m_stCBufferReferences; }

        /**
         * 增加 CBuffer 的引用
         * @pre !ContainsSymbol(cb->GetName())
         * @param cb CBuffer 对象
         */
        void AddConstantBufferReference(ImmutableConstantBufferDefinitionPtr cb);

        /**
         * 获取纹理引用列表
         */
        [[nodiscard]] const auto& GetTextureReferences() const noexcept { return m_stTextureReferences; }

        /**
         * 增加 Texture 的引用
         * @pre !ContainsSymbol(texture->GetName())
         * @param texture 纹理对象
         */
        void AddTextureReference(ImmutableShaderTextureDefinitionPtr texture);

        /**
         * 获取顶点布局
         */
        [[nodiscard]] const ImmutableShaderVertexLayoutDefinitionPtr& GetVertexLayout() const noexcept { return m_pVertexLayout; }

        /**
         * 设置顶点布局
         * @pre GetType() == ShaderTypes::VertexShader
         * @param layout 布局
         */
        void SetVertexLayout(ImmutableShaderVertexLayoutDefinitionPtr layout) noexcept;

        /**
         * 检查是否已经定义了符号
         * @param symbol 符号
         */
        [[nodiscard]] bool ContainsSymbol(std::string_view symbol) const noexcept;

        /**
         * 获取哈希值
         */
        [[nodiscard]] size_t GetHashCode() const noexcept;

    private:
        ShaderTypes m_iType;
        std::string m_stName;
        std::string m_stSource;
        std::string m_stEntry = "main";
        std::vector<ImmutableConstantBufferDefinitionPtr> m_stCBufferReferences;
        std::vector<ImmutableShaderTextureDefinitionPtr> m_stTextureReferences;
        ImmutableShaderVertexLayoutDefinitionPtr m_pVertexLayout;
    };

    using ShaderDefinitionPtr = std::shared_ptr<ShaderDefinition>;
    using ImmutableShaderDefinitionPtr = std::shared_ptr<const ShaderDefinition>;
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::Effect::ShaderDefinition>
    {
        std::size_t operator()(const lstg::Subsystem::Render::Effect::ShaderDefinition& value) const
        {
            return value.GetHashCode();
        }
    };
}
