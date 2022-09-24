/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
