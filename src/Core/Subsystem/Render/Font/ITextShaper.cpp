/**
 * @file
 * @date 2022/6/30
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
