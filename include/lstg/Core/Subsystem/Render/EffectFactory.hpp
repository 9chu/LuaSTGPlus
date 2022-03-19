/**
 * @file
 * @date 2022/3/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <set>
#include <unordered_map>
#include <unordered_set>
#include "../VirtualFileSystem.hpp"
#include "../Script/LuaState.hpp"
#include "../Script/LuaRead.hpp"
#include "RenderDevice.hpp"
#include "Effect/EffectDefinition.hpp"

namespace Diligent
{
    struct IShader;
}

namespace lstg::Subsystem::Render
{
    class EffectFactory;

    namespace detail
    {
        class ShaderSourceInputStreamFactory;

        struct EffectScriptState
        {
            EffectFactory* Factory = nullptr;
            std::set<std::string, std::less<>> SymbolCache;

            bool ContainsSymbol(std::string_view name) const noexcept
            {
                return SymbolCache.find(name) != SymbolCache.end();
            }
        };
    }

    /**
     * 效果工厂
     */
    class EffectFactory
    {
    public:
        EffectFactory(VirtualFileSystem* vfs, RenderDevice* device);
        EffectFactory(const EffectFactory&) = delete;
        EffectFactory(EffectFactory&&) noexcept = delete;
        ~EffectFactory();

    public:
        /**
         * 获取基准目录
         */
        [[nodiscard]] const std::string& GetBaseDirectory() const noexcept { return m_stBaseDirectory; }

        /**
         * 设置基准目录
         * @param dir 目录
         */
        void SetBaseDirectory(std::string dir) noexcept { m_stBaseDirectory = std::move(dir); }

        /**
         * 创建效果对象
         * @param source 源
         */
        Result<Effect::ImmutableEffectDefinitionPtr> CreateEffect(std::string_view source) noexcept;

        /**
         * 从文件创建效果对象
         * @param path 路径
         */
        Result<Effect::ImmutableEffectDefinitionPtr> CreateEffectFromFile(std::string_view path) noexcept;

        /**
         * 定义全局 CBuffer
         * @pre def.GetScope() == ConstantBufferDefinition::Scope::Global
         * @param def 定义
         */
        [[nodiscard]] Effect::ImmutableConstantBufferDefinitionPtr DefineGlobalConstantBuffer(const Effect::ConstantBufferDefinition& def);

        /**
         * 获取全局 CBuffer
         * @param name 名称
         * @return CBuffer 定义，如果不存在返回 nullptr
         */
        [[nodiscard]] Effect::ImmutableConstantBufferDefinitionPtr GetGlobalConstantBuffer(std::string_view name) const noexcept;

        /**
         * 构造 Shader 定义的全局唯一共享实例
         * @param def 原始定义
         * @return 定义对象，如果编译失败返回 nullptr 和编译器输出
         */
        std::tuple<Effect::ImmutableShaderDefinitionPtr, std::string> CompileShader(const Effect::ShaderDefinition& def);

        /**
         * 获取 Shader 缓存对象
         * @param def 定义指针
         * @return Shader 对象
         */
        Diligent::IShader* GetShaderCache(const Effect::ImmutableShaderDefinitionPtr& def) const noexcept;

    private:
        Result<Effect::ImmutableEffectDefinitionPtr> CreateEffect(std::string_view source, const char* chunkName) noexcept;

    private:
        template <typename T>
        struct GetHashCodeHasher
        {
            using Element = typename T::element_type;

            size_t operator()(const T& ptr) const noexcept
            {
                return ptr->GetHashCode();
            }

            size_t operator()(const Element& e) const noexcept
            {
                return e.GetHashCode();
            }
        };

        template <typename T>
        struct SmartPtrElementEqualTo
        {
            using Element = typename T::element_type;

            bool operator()(const T& lhs, const T& rhs) const noexcept
            {
                return lhs && rhs ? *lhs == *rhs : static_cast<bool>(lhs) == static_cast<bool>(rhs);
            }

            bool operator()(const T& lhs, const Element& e) const noexcept
            {
                return lhs && *lhs == e;
            }
        };

        VirtualFileSystem* m_pFileSystem = nullptr;
        RenderDevice* m_pRenderDevice = nullptr;
        Script::LuaState m_stState;
        detail::EffectScriptState m_stScriptState;

        // 用于 #include
        std::string m_stBaseDirectory;
        detail::ShaderSourceInputStreamFactory* m_pStreamFactory = nullptr;

        // 全局 CBuffer
        std::map<std::string, Effect::ImmutableConstantBufferDefinitionPtr, std::less<>> m_stGlobalCBuffers;

        // 定义 Cache
        std::unordered_set<
            Effect::ImmutableShaderVertexLayoutDefinitionPtr,
            GetHashCodeHasher<Effect::ImmutableShaderVertexLayoutDefinitionPtr>,
            SmartPtrElementEqualTo<Effect::ImmutableShaderVertexLayoutDefinitionPtr>
        > m_stVertexLayoutDefCache;
        std::unordered_set<
            Effect::ImmutableShaderDefinitionPtr,
            GetHashCodeHasher<Effect::ImmutableShaderDefinitionPtr>,
            SmartPtrElementEqualTo<Effect::ImmutableShaderDefinitionPtr>
        > m_stShaderDefCache;

        // Shader Cache
        std::unordered_map<
            Effect::ImmutableShaderDefinitionPtr,
            Diligent::IShader*
        > m_stShaderCache;
    };

    using EffectFactoryPtr = std::shared_ptr<EffectFactory>;
}
