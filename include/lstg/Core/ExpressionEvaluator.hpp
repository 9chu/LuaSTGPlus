/**
 * @file
 * @date 2024/2/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <vector>
#include "Result.hpp"
#include "Span.hpp"

namespace lstg
{
    /**
     * 简易表达式计算
     *
     * 基于 C-like 语法提供简单的表达式计算功能。
     * 基本类型：
     *  - Null (std::nullptr_t)
     *  - Boolean (bool)
     *  - Integer (int64_t)
     *  - Float (double)
     *  - String (std::string)
     * 基本运算符：
     *  - 一元运算符：+、-、!、~
     *  - 二元运算符：+、-、*、/、%、<<、>>、&、|、^、&&、||
     *  - 比较运算符：==、!=、<、<=、>、>=
     *  - 三元运算符：?:
     */
    class ExpressionEvaluator
    {
    public:
        enum class ValueTypes
        {
            Null,
            Boolean,
            Integer,
            Float,
            String
        };

        using Value = std::variant<std::nullptr_t, bool, int64_t, double, std::string>;

        /**
         * 运行时接口
         *
         * 由调用方提供，用于获取环境变量的值或者执行函数。
         */
        class IRuntime
        {
        public:
            IRuntime() = default;
            virtual ~IRuntime() = default;

        public:
            /**
             * 获取变量的值
             * @param name 变量名
             * @return 变量值，如果不存在，返回 null
             */
            virtual Result<Value> GetVariable(std::string_view name) noexcept = 0;

            /**
             * 执行函数
             * @param name 函数名
             * @param args 参数列表
             * @return 函数计算结果
             */
            virtual Result<Value> CallFunction(std::string_view name, Span<const Value> args) noexcept = 0;
        };

        /**
         * 解析错误
         */
        enum class CompileError
        {
            UnexpectedToken = 1,
            OpCodeOutOfIndex = 2,
        };

        /**
         * 解析错误代码分类
         */
        class CompileErrorCategory : public std::error_category
        {
        public:
            static const CompileErrorCategory& GetInstance() noexcept;

        public:
            [[nodiscard]] const char* name() const noexcept override;
            [[nodiscard]] std::string message(int ev) const override;
        };

        /**
         * 运行时错误
         */
        enum class RuntimeError
        {
            StackOverflow = 1,
            BadCast = 2,
            InvalidArithmeticType = 3,
            DivisionByZero = 4,
        };

        /**
         * 运行时错误错误分类
         */
        class RuntimeErrorCategory : public std::error_category
        {
        public:
            static const RuntimeErrorCategory& GetInstance() noexcept;

        public:
            [[nodiscard]] const char* name() const noexcept override;
            [[nodiscard]] std::string message(int ev) const override;
        };

    public:
        /**
         * 编译表达式
         * @param expression 表达式
         */
        static Result<ExpressionEvaluator> Compile(std::string_view expression) noexcept;

    protected:
        ExpressionEvaluator(std::vector<uint16_t> bytecodes, std::vector<Value> constants) noexcept;

    public:
        ExpressionEvaluator(const ExpressionEvaluator&) = default;
        ExpressionEvaluator(ExpressionEvaluator&&) noexcept = default;
        ~ExpressionEvaluator() = default;

    public:
        /**
         * 计算表达式
         * @param runtime 运行时接口
         * @param stackSize 栈大小
         */
        Result<Value> Evaluate(IRuntime* runtime, size_t stackSize=10) const noexcept;

    private:
        std::vector<uint16_t> m_stBytecodes;
        std::vector<Value> m_stConstants;
    };

    inline std::error_code make_error_code(ExpressionEvaluator::CompileError ec) noexcept
    {
        return { static_cast<int>(ec), ExpressionEvaluator::CompileErrorCategory::GetInstance() };
    }

    inline std::error_code make_error_code(ExpressionEvaluator::RuntimeError ec) noexcept
    {
        return { static_cast<int>(ec), ExpressionEvaluator::RuntimeErrorCategory::GetInstance() };
    }
} // namespace lstg


namespace std
{
    template <>
    struct is_error_code_enum<lstg::ExpressionEvaluator::CompileError> : true_type {};

    template <>
    struct is_error_code_enum<lstg::ExpressionEvaluator::RuntimeError> : true_type {};
}
