/**
 * @file
 * @date 2022/3/24
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/GraphDef/DefinitionError.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::GraphDef;

const DefinitionErrorCategory& DefinitionErrorCategory::GetInstance() noexcept
{
    static const DefinitionErrorCategory kInstance;
    return kInstance;
}

const char* DefinitionErrorCategory::name() const noexcept
{
    return "DefinitionError";
}

std::string DefinitionErrorCategory::message(int ev) const
{
    switch (static_cast<DefinitionError>(ev))
    {
        case DefinitionError::Ok:
            return "ok";
        case DefinitionError::InvalidIdentifier:
            return "invalid identifier";
        case DefinitionError::SymbolAlreadyDefined:
            return "symbol already defined";
        case DefinitionError::SlotAlreadyDefined:
            return "slot already defined";
        case DefinitionError::SymbolNotFound:
            return "symbol not found";
        case DefinitionError::SymbolTypeMismatched:
            return "symbol type mismatched";
        case DefinitionError::ShaderCompileError:
            return "fail to compile shader";
        case DefinitionError::CreatePRSError:
            return "fail to create PRS";
        case DefinitionError::CreateSRBError:
            return "fail to create SRB";
        default:
            return "<unknown>";
    }
}
