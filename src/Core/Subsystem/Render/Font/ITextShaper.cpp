/**
 * @file
 * @date 2022/6/30
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Font/ITextShaper.hpp>

#include "HarfBuzzTextShaper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;

TextShaperPtr lstg::Subsystem::Render::Font::CreateHarfBuzzTextShaper()
{
    return static_pointer_cast<ITextShaper>(make_shared<HarfBuzzTextShaper>());
}
