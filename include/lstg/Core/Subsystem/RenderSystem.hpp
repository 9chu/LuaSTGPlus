/**
 * @file
 * @date 2022/2/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "../Span.hpp"
#include "ISubsystem.hpp"
#include "WindowSystem.hpp"
#include "VirtualFileSystem.hpp"
#include "Render/RenderDevice.hpp"
#include "Render/EffectFactory.hpp"
#include "Render/Mesh.hpp"
#include "Render/Camera.hpp"
#include "Render/Material.hpp"
#include "Render/GraphicsDefinitionCache.hpp"
#include "Render/IEffectPassGroupSelector.hpp"
#include "Render/Texture.hpp"
#include "Render/Texture2DData.hpp"

namespace lstg::Subsystem
{
    /**
     * 渲染子系统
     */
    class RenderSystem :
        public ISubsystem
    {
    public:
        RenderSystem(SubsystemContainer& container);
        RenderSystem(const RenderSystem&) = delete;
        RenderSystem(RenderSystem&&) = delete;
        ~RenderSystem() = default;

    public:
        /**
         * 获取渲染设备
         */
        [[nodiscard]] Render::RenderDevice* GetRenderDevice() const noexcept { return m_pRenderDevice.get(); }

        /**
         * 获取效果工厂
         */
        [[nodiscard]] Render::EffectFactory* GetEffectFactory() const noexcept { return m_pEffectFactory.get(); }

        // <editor-fold desc="资源分配">
    public:
        /**
         * 创建静态网格
         * @param def 定义
         * @param vertexData 顶点数据
         * @param indexData 索引数据
         * @return 网格指针
         */
        [[nodiscard]] Result<Render::MeshPtr> CreateStaticMesh(const Render::GraphDef::MeshDefinition& def, Span<const uint8_t> vertexData,
            Span<const uint16_t> indexData) noexcept
        {
            return CreateStaticMesh(def, vertexData, {reinterpret_cast<const uint8_t*>(indexData.data()), indexData.size() * 2}, false);
        }

        /**
         * 创建静态网格
         * @param def 定义
         * @param vertexData 顶点数据
         * @param indexData 索引数据
         * @return 网格指针
         */
        [[nodiscard]] Result<Render::MeshPtr> CreateStaticMesh(const Render::GraphDef::MeshDefinition& def, Span<const uint8_t> vertexData,
            Span<const uint32_t> indexData) noexcept
        {
            return CreateStaticMesh(def, vertexData, {reinterpret_cast<const uint8_t*>(indexData.data()), indexData.size() * 4}, true);
        }

        // TODO: CreateDynamicMesh

        /**
         * 创建相机
         */
        [[nodiscard]] Result<Render::CameraPtr> CreateCamera() noexcept;

        /**
         * 创建材质
         * @param effect 特效对象
         * @return 材质对象
         */
        [[nodiscard]] Result<Render::MaterialPtr> CreateMaterial(const Render::GraphDef::ImmutableEffectDefinitionPtr& effect) noexcept;

        /**
         * 创建 2D 纹理
         * @param data 纹理数据
         * @return 纹理对象
         */
        [[nodiscard]] Result<Render::TexturePtr> CreateTexture2D(const Render::Texture2DData& data) noexcept;

    private:
        [[nodiscard]] Result<Render::MeshPtr> CreateStaticMesh(const Render::GraphDef::MeshDefinition& def, Span<const uint8_t> vertexData,
            Span<const uint8_t> indexData, bool use32BitIndex) noexcept;

        // </editor-fold>
        // <editor-fold desc="渲染控制">
    public:
        /**
         * 开始一帧
         */
        Result<void> BeginFrame() noexcept;

        /**
         * 结束一帧
         */
        void EndFrame() noexcept;

        /**
         * 获取相机
         */
        [[nodiscard]] Render::CameraPtr GetCamera() const noexcept { return m_pCurrentCamera; }

        /**
         * 设置相机
         * @param camera 相机
         */
        void SetCamera(Render::CameraPtr camera) noexcept;

        /**
         * 获取材质
         */
        [[nodiscard]] Render::MaterialPtr GetMaterial() const noexcept { return m_pCurrentMaterial; }

        /**
         * 设置材质
         * @param material 材质
         */
        void SetMaterial(Render::MaterialPtr material) noexcept;

        /**
         * 获取效果组选择器
         */
        [[nodiscard]] Render::EffectPassGroupSelectorPtr GetEffectPassGroupSelector() const noexcept { return m_pCurrentSelector; }

        /**
         * 设置效果组选择器
         * @param selector 选择器
         */
        void SetEffectPassGroupSelector(Render::EffectPassGroupSelectorPtr selector) noexcept;

        /**
         * 绘制网格
         * @param mesh 网格对象
         * @param indexCount 使用的索引个数
         * @param indexOffset 索引偏移（个数）
         * @param vertexOffset 顶点偏移（个数），offset = vertexOffset * stride
         */
        Result<void> Draw(Render::Mesh* mesh, size_t indexCount, size_t indexOffset, size_t vertexOffset = 0) noexcept;

    private:
        std::tuple<uint32_t, uint32_t> GetCurrentOutputViewSize() noexcept;
        const Render::GraphDef::EffectPassGroupDefinition* SelectPassGroup() noexcept;
        Result<void> CommitCamera() noexcept;
        Result<void> CommitMaterial() noexcept;
        Result<void> PreparePipeline(const Render::GraphDef::EffectPassDefinition* pass, const Render::GraphDef::MeshDefinition* meshDef);

        // </editor-fold>
    protected:  // ISubsystem
        void OnEvent(SubsystemEvent& event) noexcept override;

    private:
        std::shared_ptr<WindowSystem> m_pWindowSystem;
        std::shared_ptr<VirtualFileSystem> m_pVirtualFileSystem;
        Render::RenderDevicePtr m_pRenderDevice;
        Render::EffectFactoryPtr m_pEffectFactory;

        // 缓存
        Render::GraphicsDefinitionCache<Render::GraphDef::MeshDefinition> m_stMeshDefCache;

        // 内建全局 CBuffer
        Render::ConstantBufferPtr m_pCameraStateCBuffer;

        // 内建默认纹理
        Render::TexturePtr m_pDefaultTexture2D;

        // 渲染状态
        Render::CameraPtr m_pCurrentCamera;
        Render::MaterialPtr m_pCurrentMaterial;
        Render::EffectPassGroupSelectorPtr m_pCurrentSelector;
        const Render::GraphDef::EffectPassGroupDefinition* m_pCurrentPassGroup = nullptr;
        Render::Camera::Viewport m_stCurrentViewport;
        Render::Camera::OutputViews m_stCurrentOutputViews;
    };
}
