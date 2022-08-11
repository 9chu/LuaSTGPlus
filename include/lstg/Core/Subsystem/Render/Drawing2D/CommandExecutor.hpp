/**
 * @file
 * @date 2022/4/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "../Material.hpp"
#include "../Mesh.hpp"
#include "CommandBuffer.hpp"
#include "../../../LRUCache.hpp"

namespace lstg::Subsystem::Render::Drawing2D
{
    /**
     * 命令执行器
     */
    class CommandExecutor
    {
    public:
        CommandExecutor(RenderSystem& renderSystem);

    public:
        /**
         * 执行渲染命令
         * @param drawData 渲染数据
         * @return 是否成功
         */
        Result<void> Execute(CommandBuffer::DrawData& drawData) noexcept;

    protected:
        virtual const GraphDef::EffectPassGroupDefinition* OnSelectEffectGroup(const GraphDef::EffectDefinition* effect,
            const std::map<std::string, std::string, std::less<>>& tags) noexcept;
        virtual void OnDrawGroup(CommandBuffer::DrawData& drawData, CommandBuffer::CommandGroup& groupData) noexcept;
        virtual void OnDrawQueue(CommandBuffer::DrawData& drawData, CommandBuffer::CommandQueue& queueData) noexcept;

    protected:
        struct SelectableEffectPassGroups
        {
            const GraphDef::EffectPassGroupDefinition* AlphaBlendGroup[2] = { nullptr, nullptr };
            const GraphDef::EffectPassGroupDefinition* AddBlendGroup[2] = { nullptr, nullptr };
            const GraphDef::EffectPassGroupDefinition* SubtractBlendGroup[2] = { nullptr, nullptr };
            const GraphDef::EffectPassGroupDefinition* ReverseSubtractBlendGroup[2] = { nullptr, nullptr };
        };

        RenderSystem& m_stRenderSystem;
        Render::TexturePtr m_pDefaultTexture;
        Render::MaterialPtr m_pDefaultMaterial;
        Render::MeshPtr m_pMesh;

        // 临时变量
        std::string m_stOldBlendTag;
        std::string m_stOldDepthDisabledTag;

        // 效果选择器
        LRUCache<const GraphDef::EffectDefinition*, SelectableEffectPassGroups, 16> m_stEffectGroupSelector;
    };
}
