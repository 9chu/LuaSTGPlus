/**
 * @file
 * @date 2022/6/30
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Font/IFontFactory.hpp>

#include "FreeTypeFontFactory.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;

FontFactoryPtr lstg::Subsystem::Render::Font::CreateFreeTypeFactory()
{
    return static_pointer_cast<IFontFactory>(make_shared<FreeTypeFontFactory>());
}
