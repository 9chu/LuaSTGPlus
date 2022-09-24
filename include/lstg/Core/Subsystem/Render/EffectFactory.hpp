/**
 * @file
 * @date 2022/3/15
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <set>
#include <unordered_map>
#include "ConstantBuffer.hpp"
#include "GraphDef/EffectDefinition.hpp"
#include "../Script/LuaState.hpp"

// Subsystem 前向声明
namespace lstg::Subsystem
{
    class RenderSystem;
    class VirtualFileSystem;
}

// ShaderSourceInputStreamFactory 前向声明
namespace lstg::Subsystem::Render::detail
{
    class DiligentShaderSourceInputStreamFactory;
}

// LuaEffectBuilder 前向声明
namespace lstg::Subsystem::Render::detail::LuaEffectBuilder
{
    struct BuilderGlobalState;
    class EffectPassBuilder;
    class ShaderBuilder;
}

namespace lstg::Subsystem::Render
{
    /**
     * 效果工厂
     */
    class EffectFactory
    {
        friend class lstg::Subsystem::Render::detail::LuaEffectBuilder::EffectPassBuilder;
        friend class lstg::Subsystem::Render::detail::LuaEffectBuilder::ShaderBuilder;

    public:
        EffectFactory(VirtualFileSystem& vfs, RenderDevice& device);
        EffectFactory(const EffectFactory&) = delete;
        EffectFactory(EffectFactory&&) noexcept = delete;
        ~EffectFactory();

    public:
        /**
         * 创建效果对象
         * @param source 源
         */
        Result<GraphDef::ImmutableEffectDefinitionPtr> CreateEffect(std::string_view source) noexcept;

        /**
         * 从文件创建效果对象
         * @param path 路径
         */
        Result<GraphDef::ImmutableEffectDefinitionPtr> CreateEffectFromFile(std::string_view path) noexcept;

        /**
         * 定义全局 CBuffer
         * @param def 定义
         * @return 是否成功
         */
        [[nodiscard]] Result<void> RegisterGlobalConstantBuffer(const ConstantBufferPtr& buf) noexcept;

        /**
         * 获取全局 CBuffer
         * @param name 名称
         * @return CBuffer 定义，如果不存在返回 nullptr
         */
        [[nodiscard]] ConstantBufferPtr GetGlobalConstantBuffer(std::string_view name) const noexcept;

    private:  // internal
        Result<GraphDef::ImmutableEffectDefinitionPtr> CreateEffect(std::string_view source, const char* chunkName) noexcept;
        Result<GraphDef::ImmutableShaderDefinitionPtr> CompileShader(const GraphDef::ShaderDefinition& def, const char* basePath) noexcept;
        Result<GraphDef::ImmutableEffectPassDefinitionPtr> CompilePass(const GraphDef::EffectPassDefinition& def) noexcept;

    private:
        VirtualFileSystem& m_stFileSystem;
        RenderDevice& m_stRenderDevice;

        Script::LuaState m_stState;
        detail::LuaEffectBuilder::BuilderGlobalState* m_pScriptState = nullptr;
        detail::DiligentShaderSourceInputStreamFactory* m_pStreamFactory = nullptr;

        // 全局 CBuffer
        std::map<std::string, ConstantBufferPtr, std::less<>> m_stGlobalCBuffers;
    };

    using EffectFactoryPtr = std::shared_ptr<EffectFactory>;
}
