/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string_view>
#include <unicode/utypes.h>
#include <unicode/uchriter.h>
#include <unicode/uobject.h>

namespace lstg::detail
{
    /**
     * ICU 字符迭代器封装
     */
    class IcuCharacterIteratorBridge :
        public icu::UCharCharacterIterator
    {
    public:
        static UClassID getStaticClassID();

    public:
        explicit IcuCharacterIteratorBridge(std::u16string_view view) noexcept;

        // Non-copyable
        IcuCharacterIteratorBridge& operator=(const IcuCharacterIteratorBridge&) = delete;

    protected:
        IcuCharacterIteratorBridge(const IcuCharacterIteratorBridge& org) noexcept;

    public:  // icu::UCharCharacterIterator
        [[nodiscard]] UClassID getDynamicClassID() const override;
        [[nodiscard]] icu::UCharCharacterIterator* clone() const override;

    private:
        std::u16string_view m_stView;
    };
}
