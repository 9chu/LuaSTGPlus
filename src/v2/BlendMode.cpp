/**
 * @file
 * @author 9chu
 * @date 2022/6/1
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/BlendMode.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2;

BlendMode::BlendMode(std::string_view str) noexcept
{
    auto nsep = str.find('+');
    if (nsep == string_view::npos)
        return;

    auto partVertex = str.substr(0, nsep);
    auto partColor = str.substr(nsep + 1);

    VertexColorBlendMode vertexBlend;
    if (partVertex == "add")
        vertexBlend = VertexColorBlendMode::Additive;
    else if (partVertex == "mul")
        vertexBlend = VertexColorBlendMode::Multiply;
    else
        return;

    ColorBlendMode colorBlend;
    if (partColor == "alpha")
        colorBlend = ColorBlendMode::Alpha;
    else if (partColor == "add")
        colorBlend = ColorBlendMode::Add;
    else if (partColor == "sub")
        colorBlend = ColorBlendMode::Subtract;
    else if (partColor == "rev")
        colorBlend = ColorBlendMode::ReverseSubtract;
    else
        return;

    VertexColorBlend = vertexBlend;
    ColorBlend = colorBlend;
}

const char* BlendMode::ToString() noexcept
{
    switch (VertexColorBlend)
    {
        case VertexColorBlendMode::Additive:
            switch (ColorBlend)
            {
                case ColorBlendMode::Alpha:
                    return "add+alpha";
                case ColorBlendMode::Add:
                    return "add+add";
                case ColorBlendMode::Subtract:
                    return "add+sub";
                case ColorBlendMode::ReverseSubtract:
                    return "add+rev";
                default:
                    return "";
            }
            break;
        case VertexColorBlendMode::Multiply:
            switch (ColorBlend)
            {
                case ColorBlendMode::Alpha:
                    return "mul+alpha";
                case ColorBlendMode::Add:
                    return "mul+add";
                case ColorBlendMode::Subtract:
                    return "mul+sub";
                case ColorBlendMode::ReverseSubtract:
                    return "mul+rev";
                default:
                    return "";
            }
        default:
            assert(false);
            return "";
    }
}
