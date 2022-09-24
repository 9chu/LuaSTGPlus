/**
 * @file
 * @date 2022/6/30
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/Font/IFontFactory.hpp>

#include "FreeTypeFontFactory.hpp"
#include "HgeFontFactory.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;

FontFactoryPtr lstg::Subsystem::Render::Font::CreateFreeTypeFactory()
{
    return static_pointer_cast<IFontFactory>(make_shared<FreeTypeFontFactory>());
}

FontFactoryPtr lstg::Subsystem::Render::Font::CreateHgeFontFactory()
{
    return static_pointer_cast<IFontFactory>(make_shared<HgeFontFactory>());
}
