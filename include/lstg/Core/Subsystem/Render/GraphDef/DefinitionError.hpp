/**
 * @file
 * @date 2022/3/24
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <system_error>

namespace lstg::Subsystem::Render::GraphDef
{
    /**
     * 定义相关错误
     */
    enum class DefinitionError
    {
        Ok = 0,
        InvalidIdentifier = 1,
        SymbolAlreadyDefined = 2,
        SlotAlreadyDefined = 3,
        SymbolNotFound = 4,
        SymbolTypeMismatched = 5,
        ShaderCompileError = 6,
        CreatePRSError = 7,
        CreateSRBError = 8,
    };

    /**
     * 定义相关错误分类
     */
    class DefinitionErrorCategory :
        public std::error_category
    {
    public:
        static const DefinitionErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;

        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(DefinitionError ec) noexcept
    {
        return { static_cast<int>(ec), DefinitionErrorCategory::GetInstance() };
    }
}

namespace std
{
    template <>
    struct is_error_code_enum<lstg::Subsystem::Render::GraphDef::DefinitionError> : true_type {};
}
