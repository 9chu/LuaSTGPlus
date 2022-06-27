/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "IcuCharacterIteratorBridge.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::detail;

UOBJECT_DEFINE_RTTI_IMPLEMENTATION(IcuCharacterIteratorBridge)

IcuCharacterIteratorBridge::IcuCharacterIteratorBridge(std::u16string_view view) noexcept
    : m_stView(view)
{
    setText(reinterpret_cast<const UChar*>(view.data()), view.length());
}

IcuCharacterIteratorBridge::IcuCharacterIteratorBridge(const IcuCharacterIteratorBridge& org) noexcept
    : icu::UCharCharacterIterator(org), m_stView(org.m_stView)
{
    setText(reinterpret_cast<const UChar*>(m_stView.data()), m_stView.length());
}

icu::UCharCharacterIterator* IcuCharacterIteratorBridge::clone() const
{
    return new IcuCharacterIteratorBridge(*this);
}
