/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "MathAlias.hpp"
#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Exception.hpp>
#include <lstg/Core/Subsystem/VFS/OverlayFileSystem.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/CommandBuffer.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/CommandExecutor.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/TextDrawing.hpp>
#include <lstg/Core/Subsystem/Render/Font/ITextShaper.hpp>
#include <lstg/Core/Subsystem/Render/Font/DynamicFontGlyphAtlas.hpp>
#include "AssetPools.hpp"
#include "GamePlay/GameWorld.hpp"

namespace lstg::v2
{
    namespace Asset
    {
        class TextureAsset;
    }

    LSTG_DEFINE_EXCEPTION(AppInitializeFailedException);

    /**
     * 游戏程序实现
     */
    class GameApp :
        public AppBase
    {
    public:
        GameApp(int argc, char** argv);

    public:  // 资源系统
        /**
         * 加载资源包
         * @param path 路径
         * @param password 密码
         * @return 是否成功
         */
        Result<void> MountAssetPack(const char* path, std::optional<std::string_view> password) noexcept;

        /**
         * 卸载资源包
         * @param path 路径
         * @return 是否成功
         */
        Result<void> UnmountAssetPack(const char* path) noexcept;

        /**
         * 获取资源池
         */
        AssetPools* GetAssetPools() const noexcept { return m_pAssetPools.get(); }

    public:  // 渲染系统
        /**
         * 获取原生分辨率
         */
        glm::vec2 GetNativeResolution() const noexcept;

        /**
         * 获取期望的分辨率
         */
        glm::vec2 GetDesiredResolution() const noexcept;

        /**
         * 获取可视范围
         */
        Math::ImageRectangleFloat GetViewportBound() const noexcept;

        /**
         * 调整目标分辨率
         * @param width 宽度
         * @param height 高度
         */
        void ChangeDesiredResolution(uint32_t width, uint32_t height) noexcept;

        /**
         * 切换全屏/窗口模式
         * @param fullscreen 是否全屏
         */
        void ToggleFullScreen(bool fullscreen) noexcept;

        /**
         * 获取渲染命令队列
         */
        Subsystem::Render::Drawing2D::CommandBuffer& GetCommandBuffer() noexcept;

        /**
         * 获取字形缓存
         */
        Subsystem::Render::Drawing2D::TextDrawing::ShapedTextCache& GetShapedTextCache() noexcept { return m_stShapedTextCache; }

        /**
         * 获取字体整形器
         */
        Subsystem::Render::Font::ITextShaper* GetTextShaper() noexcept { return m_pTextShaper.get(); }

        /**
         * 获取动态字形图集
         */
        Subsystem::Render::Font::DynamicFontGlyphAtlas* GetFontGlyphAtlas() noexcept { return m_pFontGlyphAtlas.get(); }

        /**
         * 推入 RT
         */
        Result<void> PushRenderTarget(Subsystem::Render::TexturePtr rt) noexcept;

        /**
         * 弹出 RT
         */
        Result<Subsystem::Render::TexturePtr> PopRenderTarget() noexcept;

        /**
         * 检查 RT 是否在栈上
         */
        bool IsRenderTargetInStack(Subsystem::Render::Texture* rt) noexcept;

        /**
         * 获取默认后备 RT
         */
        Result<Subsystem::Render::TexturePtr> GetDefaultRenderTarget() noexcept;

    public:  // 输入系统
        /**
         * 获取最后一次输入的字符
         */
        const std::string& GetLastInputChar() const noexcept { return m_stLastInputChar; }

        /**
         * 获取最后一次输入的键代码
         */
        int32_t GetLastInputKeyCode() const noexcept;

        /**
         * 检查按键是否按下
         * @param keyCode 键代码
         */
        bool IsKeyDown(int32_t keyCode) const noexcept;

    public:  // GamePlay
        /**
         * 获取游戏世界
         */
        GamePlay::GameWorld& GetDefaultWorld() noexcept { return m_stDefaultWorld; }
        const GamePlay::GameWorld& GetDefaultWorld() const noexcept { return m_stDefaultWorld; }
        
    protected:  // 框架事件
        void OnEvent(Subsystem::SubsystemEvent& event) noexcept override;
        void OnUpdate(double elapsed) noexcept override;
        void OnRender(double elapsed) noexcept override;

    private:
        /**
         * 根据设定调整自适应 VP 大小
         */
        void AdjustViewport() noexcept;

    private:
        // 资源系统
        std::shared_ptr<Subsystem::VFS::OverlayFileSystem> m_pAssetsFileSystem;
        AssetPoolsPtr m_pAssetPools;
        Subsystem::Asset::AssetPoolPtr m_pInternalAssetPool;  // 仅内部使用

        // 渲染
        glm::vec2 m_stNativeSolution;  // 原生分辨率
        glm::vec2 m_stDesiredSolution;  // 设计分辨率
        Math::ImageRectangleFloat m_stViewportBound;  // 视口范围
        Subsystem::Render::Drawing2D::CommandBuffer m_stCommandBuffer;
        Subsystem::Render::Drawing2D::CommandExecutor m_stCommandExecutor;

        // 文字渲染组件
        Subsystem::Render::Drawing2D::TextDrawing::ShapedTextCache m_stShapedTextCache;
        Subsystem::Render::Font::TextShaperPtr m_pTextShaper;
        Subsystem::Render::Font::DynamicFontGlyphAtlasPtr m_pFontGlyphAtlas;

        // RT Stack
        std::shared_ptr<Asset::TextureAsset> m_pDefaultRenderTargetAsset;
        std::vector<Subsystem::Render::TexturePtr> m_stRenderTargetStack;

        // 输入状态
        uint32_t m_uInputWindowID = 0;  // 输入窗口 ID
        std::string m_stLastInputChar;  // 最后一次输入的字符（UTF-8）
        int32_t m_iLastInputKeyCode = '\0';  // 最后一次输入的按键
        std::vector<bool> m_stKeyStateMap;  // 按键状态，使用 vector<bool> 等价于 bitmap

        // 游戏世界
        GamePlay::GameWorld m_stDefaultWorld;
    };
}
