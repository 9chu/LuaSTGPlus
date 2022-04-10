/**
 * @file
 * @date 2022/4/10
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "GraphDef/EffectPassGroupDefinition.hpp"

namespace lstg::Subsystem::Render
{
    /**
     * 效果组选择器
     */
    class IEffectPassGroupSelector
    {
    public:
        /**
         * 重置内部状态
         */
        virtual void Reset() noexcept = 0;

        /**
         * 获取选中的效果组
         */
        virtual const GraphDef::EffectPassGroupDefinition* GetSelectedPassGroup() noexcept = 0;

        /**
         * 增加备选
         * @param group 组
         */
        virtual void AddCandidate(const GraphDef::EffectPassGroupDefinition* group) noexcept = 0;
    };

    using EffectPassGroupSelectorPtr = std::shared_ptr<IEffectPassGroupSelector>;
}
