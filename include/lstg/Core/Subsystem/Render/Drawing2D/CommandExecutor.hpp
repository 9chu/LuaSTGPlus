/**
 * @file
 * @date 2022/4/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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

        /**
         * 获取最后一次执行产生的 DrawCall 数量
         */
        size_t GetLastExecutedDrawCalls() const noexcept { return m_uDrawCalls; }

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

        // 数据统计
        size_t m_uDrawCalls = 0;
    };
}
