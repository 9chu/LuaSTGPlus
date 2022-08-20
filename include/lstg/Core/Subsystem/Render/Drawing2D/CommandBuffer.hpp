/**
 * @file
 * @date 2022/4/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <vector>
#include "../../../Flag.hpp"
#include "../../../Hash.hpp"
#include "../Camera.hpp"
#include "../Material.hpp"
#include "../ColorRGBA32.hpp"
#include "FreeList.hpp"

namespace lstg::Subsystem::Render::Drawing2D
{
    /**
     * 颜色混合模式
     */
    enum class ColorBlendMode
    {
        Alpha = 0,
        Add,
        Subtract,
        ReverseSubtract,
    };

    /**
     * 雾类型
     */
    enum class FogTypes
    {
        Disabled = 0,
        Linear,
        Exp,
        Exp2,
    };

    /**
     * 清理标志
     */
    LSTG_FLAG_BEGIN(ClearFlags)
        None = 0,
        Color = 1,
        Depth = 2,
    LSTG_FLAG_END(ClearFlags)

    /**
     * 顶点
     */
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec2 TexCoord;
        ColorRGBA32 Color0 = 0x00000000;  // 在默认2D材质中被当做 AdditiveColor
        ColorRGBA32 Color1 = 0x00000000;  // 在默认2D材质中被当做 MultiplierColor
    };

    /**
     * 命令缓冲
     */
    class CommandBuffer
    {
    public:
        struct DrawCommand
        {
            ColorBlendMode ColorBlend = ColorBlendMode::Alpha;  // 颜色混合模式
            bool NoDepth = false;  // 是否关闭深度
            FogTypes FogType = FogTypes::Disabled;  // 雾类型
            float FogArg1 = 0.f;  // 雾参数1
            float FogArg2 = 0.f;  // 雾参数2
            ColorRGBA32 FogColor = 0x00000000;  // 雾颜色
            size_t TextureId = 0;  // 纹理ID
            size_t MaterialId = 0;  // 材质ID
            size_t IndexStart = 0;  // Index起始下标
            size_t IndexCount = 0;  // Index个数
            size_t BaseVertexIndex = 0;  // 基准顶点索引
        };

        using DrawCommandContainer = std::vector<DrawCommand>;

        struct CommandQueue
        {
            size_t CameraId = 0;  // 用于渲染的相机
            ClearFlags ClearFlag = ClearFlags::None;  // 清理标记
            ColorRGBA32 ClearColor = 0x00000000;  // 清理颜色
            DrawCommandContainer Commands;  // 绘图命令
        };

        using CommandQueueContainer = std::vector<FreeListPtr<CommandQueue>>;

        struct CommandGroup
        {
            uint64_t GroupId;
            CommandQueueContainer Queue;
        };

        using CommandGroupContainer = std::vector<FreeListPtr<CommandGroup>>;

        struct DrawData
        {
            CommandGroupContainer& CommandGroup;
            std::vector<Vertex>& VertexBuffer;
            std::vector<uint16_t>& IndexBuffer;
            std::vector<FreeListPtr<CameraPtr>>& CameraList;
            std::vector<TexturePtr>& TextureList;
            std::vector<MaterialPtr>& MaterialList;
        };

    public:
        CommandBuffer();

    public:
        /**
         * 清空所有资源和引用
         */
        void Begin() noexcept;

        /**
         * 完成命令队列收集
         */
        DrawData End() noexcept;

        /**
         * 通过 ID 查询缓存的纹理
         * @param id ID
         * @return 纹理对象
         */
        [[nodiscard]] TexturePtr FindTextureById(size_t id) const noexcept;

        /**
         * 通过 ID 查询缓存的材质
         * @param id ID
         * @return 材质对象
         */
        [[nodiscard]] MaterialPtr FindMaterialById(size_t id) const noexcept;

        /**
         * 设置渲染组 ID
         * @param id ID
         */
        void SetGroupId(uint64_t id) noexcept;

        /**
         * 设置观察矩阵
         * @param matrix 矩阵
         */
        void SetView(glm::mat4x4 matrix) noexcept;

        /**
         * 设置投影矩阵
         * @param matrix 矩阵
         */
        void SetProjection(glm::mat4x4 matrix) noexcept;

        /**
         * 设置视口
         * @param l X
         * @param t Y
         * @param w 宽度
         * @param h 高度
         */
        void SetViewport(float l, float t, float w, float h) noexcept;

        /**
         * 设置输出缓冲
         * @param colorView 颜色缓冲
         * @param depthStencilView 深度缓冲
         */
        void SetOutputViews(TexturePtr colorView, TexturePtr depthStencilView) noexcept;

        /**
         * 获取当前的材质
         */
        const MaterialPtr& GetMaterial() const noexcept { return m_pCurrentMaterial; }

        /**
         * 设置材质
         * @param material 材质
         */
        void SetMaterial(MaterialPtr material) noexcept;

        /**
         * 设置颜色混合模式
         * @param m 混合模式
         */
        void SetColorBlendMode(ColorBlendMode m) noexcept;

        /**
         * 设置是否关闭深度
         * @param b 是否关闭深度
         */
        void SetNoDepth(bool b) noexcept;

        /**
         * 设置雾
         * @param fog 雾类型
         * @param fogColor 雾颜色
         * @param arg1 参数1
         * @param arg2 参数2
         */
        void SetFog(FogTypes fog, ColorRGBA32 fogColor, float arg1, float arg2) noexcept;

        /**
         * 绘制四边形
         * @param tex2d 关联的纹理
         * @param arr 顶点数组
         * @return 是否成功
         */
        Result<void> DrawQuad(TexturePtr tex2d, const Vertex (&arr)[4]) noexcept;

        /**
         * 绘制四边形
         * @param tex2d 关联的纹理
         * @return 如果成功，返回四边形顶点指针，否则返回错误
         */
        Result<Span<Vertex>> DrawQuadInPlace(TexturePtr tex2d) noexcept;

        /**
         * 清空颜色和 ZBuffer
         * @param color 颜色
         * @return 是否成功
         */
        Result<void> Clear(ColorRGBA32 color) noexcept;

    private:
        void PrepareNewGroup() noexcept;
        void PrepareNewQueue() noexcept;
        void PrepareNewCommand() noexcept;
        size_t AllocCamera();
        Result<size_t> AllocTexture(TexturePtr tex2d) noexcept;
        Result<size_t> AllocMaterial(MaterialPtr mat) noexcept;
        Result<void> InstantialGroup() noexcept;
        Result<void> InstantialQueue() noexcept;
        Result<void> InstantialCommand() noexcept;

    private:
        struct CameraStateKey
        {
            mutable std::optional<uint32_t> Hash;

            glm::mat4x4 View;
            glm::mat4x4 Projection;
            Camera::Viewport Viewport;
            Camera::OutputViews OutputViews;

            bool operator==(const CameraStateKey& rhs) const noexcept
            {
                return View == rhs.View && Projection == rhs.Projection && Viewport == rhs.Viewport && OutputViews == rhs.OutputViews;
            }
        };

        struct CameraStateKeyHasher
        {
            size_t operator()(const CameraStateKey& key) const noexcept
            {
                if (key.Hash)
                    return *key.Hash;
                key.Hash = MurmurHash3({reinterpret_cast<const uint8_t*>(&key.Projection), sizeof(key.Projection)}) ^
                    MurmurHash3({reinterpret_cast<const uint8_t*>(&key.Viewport), sizeof(key.Viewport)}) ^
                    static_cast<uint32_t>(std::hash<void*>{}(key.OutputViews.ColorView.get())) ^
                    static_cast<uint32_t>(std::hash<void*>{}(key.OutputViews.DepthStencilView.get()));
                return *key.Hash;
            }
        };

        using UniqueCameraCache = std::unordered_map<CameraStateKey, size_t, CameraStateKeyHasher>;

        // 资源池
        FreeList<CameraPtr> m_stCameraFreeList;
        FreeList<CommandGroup> m_stCommandGroupFreeList;
        FreeList<CommandQueue> m_stCommandQueueFreeList;

        // 引用资源映射
        std::vector<FreeListPtr<CameraPtr>> m_stCameraReferences;
        UniqueCameraCache m_stCameraMapping;
        std::vector<TexturePtr> m_stTextureReferences;
        std::map<Texture*, size_t> m_stTextureMapping;
        std::vector<MaterialPtr> m_stMaterialReferences;
        std::map<Material*, size_t> m_stMaterialMapping;

        // 正在生成的图元
        size_t m_uCurrentBaseVertexIndex = 0;
        std::vector<Vertex> m_stVertices;
        std::vector<uint16_t> m_stIndexes;

        // 正在生成的命令组
        CommandGroupContainer m_stCommandGroups;

        // 当前状态
        std::optional<CommandGroupContainer::iterator> m_stCurrentGroup;
        std::optional<CommandQueueContainer::iterator> m_stCurrentQueue;
        std::optional<DrawCommandContainer::iterator> m_stCurrentDrawCommand;
        uint64_t m_ullCurrentGroupId = 0;
        glm::mat4x4 m_stCurrentView;
        glm::mat4x4 m_stCurrentProjection;
        Camera::Viewport m_stCurrentViewport;
        Camera::OutputViews m_stCurrentOutputViews;
        MaterialPtr m_pCurrentMaterial;
        ColorBlendMode m_iCurrentColorBlendMode = ColorBlendMode::Alpha;
        bool m_bNoDepth = false;
        FogTypes m_iCurrentFogType = FogTypes::Disabled;
        float m_fCurrentFogArg1 = 0.f;
        float m_fCurrentFogArg2 = 0.f;
        ColorRGBA32 m_stCurrentFogColor = 0x00000000;  // 雾颜色
    };
}
