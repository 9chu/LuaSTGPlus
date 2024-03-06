/**
 * @file
 * @date 2024/2/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/ExpressionEvaluator.hpp>

#include <cmath>
#include <cstring>
#include <algorithm>
#include <vector>
#include <optional>
#include <fmt/format.h>
#include <stb_c_lexer.h>

using namespace std;
using namespace lstg;

namespace
{
    struct Token
    {
        using TokenLiteral = std::variant<std::monostate, long, double, std::string_view>;

        long TokenType = 0;
        TokenLiteral TokenValue;
        const char* FirstCharacter = nullptr;
        const char* LastCharacter = nullptr;

        template <size_t Size>
        static void PrintToken(char (&buffer)[Size], long tokenType) noexcept
        {
            static_assert(Size > 8, "buffer is too small");

            ::memset(buffer, 0, Size);
            switch (tokenType)
            {
                case CLEX_eof:
                    ::strcat(buffer, "<eof>");
                    break;
                case CLEX_parse_error:
                    ::strcat(buffer, "<error>");
                    break;
                case CLEX_intlit:
                    ::strcat(buffer, "<int>");
                    break;
                case CLEX_floatlit:
                    ::strcat(buffer, "<float>");
                    break;
                case CLEX_id:
                    ::strcat(buffer, "<id>");
                    break;
                case CLEX_dqstring:
                case CLEX_sqstring:
                    ::strcat(buffer, "<string>");
                    break;
                case CLEX_charlit:
                    ::strcat(buffer, "<char>");
                    break;
                case CLEX_eq:
                    ::strcat(buffer, "'=='");
                    break;
                case CLEX_noteq:
                    ::strcat(buffer, "'!='");
                    break;
                case CLEX_lesseq:
                    ::strcat(buffer, "'<='");
                    break;
                case CLEX_greatereq:
                    ::strcat(buffer, "'>='");
                    break;
                case CLEX_andand:
                    ::strcat(buffer, "'&&'");
                    break;
                case CLEX_oror:
                    ::strcat(buffer, "'||'");
                    break;
                case CLEX_shl:
                    ::strcat(buffer, "'<<'");
                    break;
                case CLEX_shr:
                    ::strcat(buffer, "'>>'");
                    break;
                case CLEX_plusplus:
                    ::strcat(buffer, "'++'");
                    break;
                case CLEX_minusminus:
                    ::strcat(buffer, "'--'");
                    break;
                case CLEX_pluseq:
                    ::strcat(buffer, "'+='");
                    break;
                case CLEX_minuseq:
                    ::strcat(buffer, "'-='");
                    break;
                case CLEX_muleq:
                    ::strcat(buffer, "'*='");
                    break;
                case CLEX_diveq:
                    ::strcat(buffer, "'/='");
                    break;
                case CLEX_modeq:
                    ::strcat(buffer, "'%='");
                    break;
                case CLEX_andeq:
                    ::strcat(buffer, "'&='");
                    break;
                case CLEX_oreq:
                    ::strcat(buffer, "'|='");
                    break;
                case CLEX_xoreq:
                    ::strcat(buffer, "'^='");
                    break;
                case CLEX_arrow:
                    ::strcat(buffer, "'->'");
                    break;
                case CLEX_eqarrow:
                    ::strcat(buffer, "'=>'");
                    break;
                case CLEX_shleq:
                    ::strcat(buffer, "'<<='");
                    break;
                case CLEX_shreq:
                    ::strcat(buffer, "'>>='");
                    break;
                default:
                    if (tokenType < 128 && ::isprint(tokenType))
                        ::snprintf(buffer, Size, "'%c'", static_cast<char>(tokenType));
                    else
                        ::snprintf(buffer, Size, "\\%d", static_cast<int>(tokenType));
                    break;
            }
        }

        static int GetBinaryOpPriority(long tokenType) noexcept
        {
            switch (tokenType)
            {
                case '*':
                case '/':
                case '%':
                    return 1;
                case '+':
                case '-':
                    return 2;
                case CLEX_shl:
                case CLEX_shr:
                    return 3;
                case '<':
                case '>':
                case CLEX_lesseq:
                case CLEX_greatereq:
                    return 4;
                case CLEX_eq:
                case CLEX_noteq:
                    return 5;
                case '&':
                    return 6;
                case '^':
                    return 7;
                case '|':
                    return 8;
                case CLEX_andand:
                    return 9;
                case CLEX_oror:
                    return 10;
                default:
                    return -1;
            }
        }

        static const int kBinaryOpPriorityMax = 10;
    };

    enum class OpCodes
    {
        Nop = 0,

        PushNull,  // -0, +1
        PushTrue,  // -0, +1
        PushFalse,  // -0, +1

        Jump,  // -0, +0
        JumpIfFalse,  // -1, +0
        JumpIfTrue,  // -1, +0

        Positive,  // -1, +1
        Negative,  // -1, +1
        BitNot,  // -1, +1
        LogicalNot,  // -1, +1

        Add,  // -2, +1
        Sub,  // -2, +1
        Mul,  // -2, +1
        Div,  // -2, +1
        Mod,  // -2, +1
        LShift,  // -2, +1
        RShift,  // -2, +1
        BitAnd,  // -2, +1
        BitXor,  // -2, +1
        BitOr,  // -2, +1
        Less,  // -2, +1
        Greater,  // -2, +1
        LessEq,  // -2, +1
        GreaterEq,  // -2, +1
        Equal,  // -2, +1
        NotEqual,  // -2, +1

        LoadConstant,  // -0, +1
        LoadValue,  // -0, +1
        CallFunction,  // -nargs, +1
    };

    class ExpressionParser
    {
    public:
        ExpressionParser(std::string_view expression)
        {
            // FIXME: 这里限定了 String 长度不超过 sizeof(m_stReadBuffer)
            ::stb_c_lexer_init(&m_stLexer, expression.data(), expression.data() + expression.size(), m_stReadBuffer,
                sizeof(m_stReadBuffer));
        }

    public:  // Error Handing
        [[nodiscard]] const std::string& GetErrorMessage() const noexcept { return m_stErrorMessage; }

        template <typename... TArgs>
        void SetErrorMessage(fmt::format_string<TArgs...> format, TArgs&&... args) noexcept
        {
            try
            {
                m_stErrorMessage.clear();
                fmt::format_to(std::back_inserter(m_stErrorMessage), format, std::forward<TArgs>(args)...);
            }
            catch (...)
            {
                m_stErrorMessage.clear();
            }
        }

    public:  // Tokenizer
        void Advance() noexcept
        {
            if (m_stCurrent && (m_stCurrent->TokenType == CLEX_eof || m_stCurrent->TokenType == CLEX_parse_error))
                return;

            Token token;
            int ret = ::stb_c_lexer_get_token(&m_stLexer);
            if (ret == 0)
            {
                token.TokenType = CLEX_eof;
                token.TokenValue = {};
                token.FirstCharacter = token.LastCharacter = m_stLexer.parse_point;
            }
            else
            {
                token.TokenType = m_stLexer.token;
                switch (token.TokenType)
                {
                    case CLEX_intlit:
                    case CLEX_charlit:
                        token.TokenValue = m_stLexer.int_number;
                        break;
                    case CLEX_floatlit:
                        token.TokenValue = m_stLexer.real_number;
                        break;
                    case CLEX_id:
                    case CLEX_dqstring:
                    case CLEX_sqstring:
                        token.TokenValue = string_view { m_stLexer.string, static_cast<size_t>(m_stLexer.string_len) };
                        break;
                    default:
                        token.TokenValue = {};
                        break;
                }
                token.FirstCharacter = m_stLexer.where_firstchar;
                token.LastCharacter = m_stLexer.where_lastchar;
            }
            m_stCurrent = std::move(token);
        }

        const Token& Current() noexcept
        {
            if (!m_stCurrent)
                Advance();
            assert(m_stCurrent);
            return *m_stCurrent;
        }

    public: // Matcher
        Result<void> AcceptToken(char singleChar) noexcept
        {
            if (Current().TokenType == singleChar)
            {
                Advance();
                return {};
            }

            char tokenBufferSeen[16];
            char tokenBufferExpected[16];
            Token::PrintToken(tokenBufferSeen, Current().TokenType);
            Token::PrintToken(tokenBufferExpected, singleChar);
            SetErrorMessage("Token {} expected, but found {}", tokenBufferExpected, tokenBufferSeen);
            return make_error_code(ExpressionEvaluator::CompileError::UnexpectedToken);
        }

        bool TryAcceptToken(char singleChar) noexcept
        {
            if (Current().TokenType == singleChar)
            {
                Advance();
                return true;
            }
            return false;
        }

    public:  // Emitter
        [[nodiscard]] size_t GetBytecodeCursor() const noexcept
        {
            return m_stBytecodes.size();
        }

        Result<void> EmitBytecode(OpCodes op, int arg) noexcept
        {
            try
            {
                m_stBytecodes.push_back(0);
            }
            catch (...)
            {
                SetErrorMessage("Out of memory");
                return make_error_code(errc::not_enough_memory);
            }

            return SetBytecode(m_stBytecodes.size() - 1, op, arg);
        }

        Result<void> SetBytecode(size_t cursor, OpCodes op, int arg) noexcept
        {
            assert(cursor < m_stBytecodes.size());
            assert(arg >= 0);

            if (arg > 255)
            {
                SetErrorMessage("Instruction {} argument out of limitation", static_cast<int>(op));
                return make_error_code(ExpressionEvaluator::CompileError::OpCodeOutOfIndex);
            }

            m_stBytecodes[cursor] = (static_cast<unsigned>(op) << 8u) | static_cast<uint8_t>(arg);
            return {};
        }

        Result<size_t> RecordConstant(Token::TokenLiteral value) noexcept
        {
            assert(value.index() != 0);

            if (value.index() == 1)  // long
            {
                auto it = std::find_if(m_stConstants.begin(), m_stConstants.end(),
                    [&](const ExpressionEvaluator::Value& e) {
                        return e.index() == 2 && std::get<int64_t>(e) == std::get<long>(value);
                    });

                if (it != m_stConstants.end())
                    return static_cast<size_t>(it - m_stConstants.begin());

                try
                {
                    m_stConstants.emplace_back(static_cast<int64_t>(std::get<long>(value)));
                    return m_stConstants.size() - 1;
                }
                catch (...)
                {
                    SetErrorMessage("Out of memory");
                    return make_error_code(errc::not_enough_memory);
                }
            }
            else if (value.index() == 2)  // double
            {
                auto it = std::find_if(m_stConstants.begin(), m_stConstants.end(),
                    [&](const ExpressionEvaluator::Value& e) {
                        return e.index() == 3 && std::get<double>(e) == std::get<double>(value);
                    });

                if (it != m_stConstants.end())
                    return static_cast<size_t>(it - m_stConstants.begin());

                try
                {
                    m_stConstants.emplace_back(std::get<double>(value));
                    return m_stConstants.size() - 1;
                }
                catch (...)
                {
                    SetErrorMessage("Out of memory");
                    return make_error_code(errc::not_enough_memory);
                }
            }
            else if (value.index() == 3)  // string_view
            {
                auto it = std::find_if(m_stConstants.begin(), m_stConstants.end(),
                    [&](const ExpressionEvaluator::Value& e) {
                        return e.index() == 4 && std::get<std::string>(e) == std::get<std::string_view>(value);
                    });

                if (it != m_stConstants.end())
                    return static_cast<size_t>(it - m_stConstants.begin());

                try
                {
                    m_stConstants.emplace_back(std::string {std::get<std::string_view>(value)});
                    return m_stConstants.size() - 1;
                }
                catch (...)
                {
                    SetErrorMessage("Out of memory");
                    return make_error_code(errc::not_enough_memory);
                }
            }

            return static_cast<size_t>(-1);
        }

        std::vector<uint16_t> GetBytecodes() noexcept { return m_stBytecodes; }

        std::vector<ExpressionEvaluator::Value> GetConstants() noexcept { return m_stConstants; }

    public:  // Syntax
        Result<void> ParseExpression() noexcept
        {
            return ParseConditionalExpression();
        }

        Result<void> ParseConditionalExpression() noexcept
        {
            // <conditional-expression> ::= <logical-or-expression>
            //                            | <logical-or-expression> ? <expression> : <conditional-expression>

            Result<void> ret = ParseBinaryExpression(Token::kBinaryOpPriorityMax + 1);
            if (!ret)
                return ret;

            if (TryAcceptToken('?'))
            {
                auto jumpInstructionAt = GetBytecodeCursor();
                ret = EmitBytecode(OpCodes::JumpIfFalse, 0);
                if (!ret)
                    return ret;

                // 真分支
                ret = ParseExpression();
                if (!ret)
                    return ret;

                auto trueBranchExitInstructionAt = GetBytecodeCursor();
                ret = EmitBytecode(OpCodes::Jump, 0);
                if (!ret)
                    return ret;

                ret = AcceptToken(':');
                if (!ret)
                    return ret;

                // 假分支
                auto falseBranchEnterInstructionAt = GetBytecodeCursor();
                ret = ParseConditionalExpression();
                if (!ret)
                    return ret;

                auto trueBranchJumpTarget = GetBytecodeCursor();

                // 修改 Bytecode
                ret = SetBytecode(jumpInstructionAt, OpCodes::JumpIfFalse,
                    static_cast<int>(falseBranchEnterInstructionAt - jumpInstructionAt));
                if (!ret)
                    return ret;

                ret = SetBytecode(trueBranchExitInstructionAt, OpCodes::Jump,
                    static_cast<int>(trueBranchJumpTarget - trueBranchExitInstructionAt));
                if (!ret)
                    return ret;
            }
            return {};
        }

        Result<void> ParseBinaryExpression(int priority) noexcept
        {
            // <logical-or-expression> ::= <logical-and-expression>
            //                           | <logical-or-expression> || <logical-and-expression>
            //
            // <logical-and-expression> ::= <inclusive-or-expression>
            //                            | <logical-and-expression> && <inclusive-or-expression>
            //
            // <inclusive-or-expression> ::= <exclusive-or-expression>
            //                             | <inclusive-or-expression> | <exclusive-or-expression>
            //
            // <exclusive-or-expression> ::= <and-expression>
            //                             | <exclusive-or-expression> ^ <and-expression>
            //
            // <and-expression> ::= <equality-expression>
            //                    | <and-expression> & <equality-expression>
            //
            // <equality-expression> ::= <relational-expression>
            //                         | <equality-expression> == <relational-expression>
            //                         | <equality-expression> != <relational-expression>
            //
            // <relational-expression> ::= <shift-expression>
            //                           | <relational-expression> < <shift-expression>
            //                           | <relational-expression> > <shift-expression>
            //                           | <relational-expression> <= <shift-expression>
            //                           | <relational-expression> >= <shift-expression>
            //
            // <shift-expression> ::= <additive-expression>
            //                      | <shift-expression> << <additive-expression>
            //                      | <shift-expression> >> <additive-expression>
            //
            // <additive-expression> ::= <multiplicative-expression>
            //                         | <additive-expression> + <multiplicative-expression>
            //                         | <additive-expression> - <multiplicative-expression>
            //
            // <multiplicative-expression> ::= <unary-expression>
            //                               | <multiplicative-expression> * <unary-expression>
            //                               | <multiplicative-expression> / <unary-expression>
            //                               | <multiplicative-expression> % <unary-expression>

            // 退化
            if (priority == 0)
                return ParseUnaryExpression();

            // 解析左侧
            auto ret = ParseBinaryExpression(priority - 1);
            if (!ret)
                return ret;

            // 检查是否为二元运算符
            auto currentToken = Current().TokenType;
            int currentOpPriority = Token::GetBinaryOpPriority(currentToken);
            if (currentOpPriority < 0) // 不是二元运算符
                return {};
            else if (currentOpPriority > priority)  // 我们只能按高优先级方向解析
                return {};
            else  // 提升优先级
                priority = currentOpPriority;

            // 特殊处理 '&&' 和 '||'
            if (currentToken == CLEX_andand || currentToken == CLEX_oror)
            {
                vector<size_t> modifyPoints;
                auto appendPendingInstruction = [&]() -> Result<void> {
                    try
                    {
                        modifyPoints.push_back(GetBytecodeCursor());
                        return EmitBytecode(OpCodes::Nop, 0);
                    }
                    catch (...)
                    {
                        SetErrorMessage("Out of memory");
                        return make_error_code(errc::not_enough_memory);
                    }
                };

                ret = appendPendingInstruction();
                if (!ret)
                    return ret.GetError();

                // 循环解析右侧的表达式
                while (true)
                {
                    // 检查下一个算符
                    if (Current().TokenType != currentToken)
                        break;

                    // 吃掉运算符
                    Advance();

                    // 获取算符右侧
                    ret = ParseBinaryExpression(priority - 1);
                    if (!ret)
                        return ret;

                    ret = appendPendingInstruction();
                    if (!ret)
                        return ret.GetError();
                }

                // 生成判断通过分支
                if (currentToken == CLEX_andand)
                {
                    ret = EmitBytecode(OpCodes::PushTrue, 0);
                }
                else
                {
                    assert(currentToken == CLEX_oror);
                    ret = EmitBytecode(OpCodes::PushFalse, 0);
                }
                if (!ret)
                    return ret;

                auto jumpOutInstructionAt = GetBytecodeCursor();
                ret = EmitBytecode(OpCodes::Jump, 0);
                if (!ret)
                    return ret;

                // 生成判断熔断分支
                auto jumpTargetInstructionAt = GetBytecodeCursor();
                if (currentToken == CLEX_andand)
                {
                    ret = EmitBytecode(OpCodes::PushFalse, 0);
                }
                else
                {
                    assert(currentToken == CLEX_oror);
                    ret = EmitBytecode(OpCodes::PushTrue, 0);
                }
                if (!ret)
                    return ret;

                SetBytecode(jumpOutInstructionAt, OpCodes::Jump, static_cast<int>(GetBytecodeCursor() - jumpOutInstructionAt));

                // 修改 Pending Instructions
                for (auto& e : modifyPoints)
                {
                    auto offset = static_cast<int>(jumpTargetInstructionAt - e);
                    if (currentToken == CLEX_andand)
                        SetBytecode(e, OpCodes::JumpIfFalse, offset);
                    else
                        SetBytecode(e, OpCodes::JumpIfTrue, offset);
                }
            }
            else
            {
                // 循环解析右侧的表达式
                while (true)
                {
                    // 检查下一个算符的优先级
                    currentToken = Current().TokenType;
                    currentOpPriority = Token::GetBinaryOpPriority(currentToken);
                    if (currentOpPriority != priority)
                        break;

                    // 吃掉运算符
                    Advance();

                    // 获取算符右侧
                    ret = ParseBinaryExpression(priority - 1);
                    if (!ret)
                        return ret;

                    // 生成二元运算
                    switch (currentToken)
                    {
                        case '*':
                            ret = EmitBytecode(OpCodes::Mul, 0);
                            break;
                        case '/':
                            ret = EmitBytecode(OpCodes::Div, 0);
                            break;
                        case '%':
                            ret = EmitBytecode(OpCodes::Mod, 0);
                            break;
                        case '+':
                            ret = EmitBytecode(OpCodes::Add, 0);
                            break;
                        case '-':
                            ret = EmitBytecode(OpCodes::Sub, 0);
                            break;
                        case CLEX_shl:
                            ret = EmitBytecode(OpCodes::LShift, 0);
                            break;
                        case CLEX_shr:
                            ret = EmitBytecode(OpCodes::RShift, 0);
                            break;
                        case '<':
                            ret = EmitBytecode(OpCodes::Less, 0);
                            break;
                        case '>':
                            ret = EmitBytecode(OpCodes::Greater, 0);
                            break;
                        case CLEX_lesseq:
                            ret = EmitBytecode(OpCodes::LessEq, 0);
                            break;
                        case CLEX_greatereq:
                            ret = EmitBytecode(OpCodes::GreaterEq, 0);
                            break;
                        case CLEX_eq:
                            ret = EmitBytecode(OpCodes::Equal, 0);
                            break;
                        case CLEX_noteq:
                            ret = EmitBytecode(OpCodes::NotEqual, 0);
                            break;
                        case '&':
                            ret = EmitBytecode(OpCodes::BitAnd, 0);
                            break;
                        case '^':
                            ret = EmitBytecode(OpCodes::BitXor, 0);
                            break;
                        case '|':
                            ret = EmitBytecode(OpCodes::BitOr, 0);
                            break;
                        default:
                            assert(false);
                            break;
                    }
                    if (!ret)
                        return ret;
                }
            }
            return {};
        }

        Result<void> ParseUnaryExpression() noexcept
        {
            // <unary-operator> ::= +
            //                    | -
            //                    | ~
            //                    | !
            //
            // <unary-expression> ::= <postfix-expression>
            //                      | <unary-operator> <unary-expression>

            if (TryAcceptToken('+'))
            {
                auto ret = ParseUnaryExpression();
                if (!ret)
                    return ret;
                return EmitBytecode(OpCodes::Positive, 0);
            }
            else if (TryAcceptToken('-'))
            {
                auto ret = ParseUnaryExpression();
                if (!ret)
                    return ret;
                return EmitBytecode(OpCodes::Negative, 0);
            }
            else if (TryAcceptToken('~'))
            {
                auto ret = ParseUnaryExpression();
                if (!ret)
                    return ret;
                return EmitBytecode(OpCodes::BitNot, 0);
            }
            else if (TryAcceptToken('!'))
            {
                auto ret = ParseUnaryExpression();
                if (!ret)
                    return ret;
                return EmitBytecode(OpCodes::LogicalNot, 0);
            }
            else
            {
                return ParsePostfixExpression();
            }
        }

        Result<void> ParsePostfixExpression() noexcept
        {
            // <callarg-expression> ::= <expression>
            //                        | <callarg-expression> , <expression>
            //
            // <postfix-expression> ::= <identifier>
            //                        | <constant>
            //                        | <string>
            //                        | ( <expression> )
            //                        | <identifier> ( <callarg-expression>? )

            // 由于我们没有函数指针这样的东西，函数调用一定呈现 id ( ... ) 形式
            if (Current().TokenType == CLEX_id)
            {
                auto id = std::get<std::string_view>(Current().TokenValue);

                // 检查是否是关键词
                if (id == "true")
                {
                    auto ret = EmitBytecode(OpCodes::PushTrue, 0);
                    Advance();
                    return ret;
                }
                else if (id == "false")
                {
                    auto ret = EmitBytecode(OpCodes::PushFalse, 0);
                    Advance();
                    return ret;
                }
                else if (id == "null")
                {
                    auto ret = EmitBytecode(OpCodes::PushNull, 0);
                    Advance();
                    return ret;
                }

                // NOTE: 这里依赖 clex 内部实现，由于解析 id 后，解析 '(' 不会修改内部缓冲，因此可以临时复用内部的 m_stReadBuffer
                // 如果读到其他的 token，则不保证 id 内容的有效性
                Advance();

                if (Current().TokenType == '(')  // 函数调用
                {
                    auto constantId = RecordConstant(id);
                    if (!constantId)
                        return constantId.GetError();

                    Advance();  // eat '('

                    auto ret = EmitBytecode(OpCodes::LoadConstant, static_cast<int>(*constantId));
                    if (!ret)
                        return ret.GetError();

                    auto nargs = 0;
                    if (!TryAcceptToken(')'))
                    {
                        while (true)
                        {
                            ++nargs;
                            ret = ParseExpression();
                            if (!ret)
                                return ret;

                            if (TryAcceptToken(','))
                                continue;

                            ret = AcceptToken(')');
                            if (!ret)
                                return ret;
                            break;
                        }
                    }

                    return EmitBytecode(OpCodes::CallFunction, nargs);
                }
                else if (Current().TokenType == CLEX_dqstring || Current().TokenType == CLEX_sqstring || Current().TokenType == CLEX_id)
                {
                    // 这些 Token 会破坏上面的假设，且一定不跟在 id 后面，提前终止

                    char tokenBufferSeen[16];
                    char tokenBufferExpected[16];
                    Token::PrintToken(tokenBufferSeen, Current().TokenType);
                    Token::PrintToken(tokenBufferExpected, '(');
                    SetErrorMessage("Token {} expected, but found {}", tokenBufferExpected, tokenBufferSeen);
                    return make_error_code(ExpressionEvaluator::CompileError::UnexpectedToken);
                }

                auto constantId = RecordConstant(id);
                if (!constantId)
                    return constantId.GetError();

                return EmitBytecode(OpCodes::LoadValue, static_cast<int>(*constantId));
            }
            else if (TryAcceptToken('('))
            {
                auto ret = ParseExpression();
                if (!ret)
                    return ret;

                ret = AcceptToken(')');
                if (!ret)
                    return ret;
                return {};
            }
            else if (Current().TokenType == CLEX_intlit || Current().TokenType == CLEX_floatlit || Current().TokenType == CLEX_sqstring ||
                Current().TokenType == CLEX_dqstring)
            {
                auto constantId = RecordConstant(Current().TokenValue);
                if (!constantId)
                    return constantId.GetError();

                Advance();
                return EmitBytecode(OpCodes::LoadConstant, static_cast<int>(*constantId));
            }
            else
            {
                char tokenBufferSeen[16];
                Token::PrintToken(tokenBufferSeen, Current().TokenType);
                SetErrorMessage("Unexpected token {}", tokenBufferSeen);
                return make_error_code(ExpressionEvaluator::CompileError::UnexpectedToken);
            }
        }

    private:
        stb_lexer m_stLexer;
        char m_stReadBuffer[1024];
        std::optional<Token> m_stCurrent;

        std::string m_stErrorMessage;

        std::vector<uint16_t> m_stBytecodes;
        std::vector<ExpressionEvaluator::Value> m_stConstants;
    };

    class ValueStack
    {
    public:
        using Value = ExpressionEvaluator::Value;

        static Result<ValueStack> Create(size_t size) noexcept
        {
            try
            {
                return ValueStack(size);
            }
            catch (...)
            {
                return make_error_code(errc::not_enough_memory);
            }
        }

    protected:
        ValueStack(size_t size)
        {
            m_stStack.resize(size);
        }

    public:
        Value& operator[](int index) noexcept
        {
            auto absIndex = ToAbsIndex(index);
            assert(absIndex < m_uStackPointer);
            return m_stStack[absIndex];
        }

    public:
        [[nodiscard]] size_t GetStackPointer() const noexcept { return m_uStackPointer; }

        Value& Top() noexcept
        {
            assert(m_uStackPointer > 0);
            return m_stStack[m_uStackPointer - 1];
        }

        Result<void> Push(Value v) noexcept
        {
            if (m_uStackPointer >= m_stStack.size())
                return make_error_code(ExpressionEvaluator::RuntimeError::StackOverflow);
            m_stStack[m_uStackPointer++] = std::move(v);
            return {};
        }

        void Pop(int count=1) noexcept
        {
            assert(m_uStackPointer >= count);
            m_uStackPointer -= count;
        }

        [[nodiscard]] size_t ToAbsIndex(int index) const noexcept
        {
            if (index < 0)
                index = std::max(0, static_cast<int>(m_stStack.size()) + index);
            else
                index = std::min(static_cast<int>(m_stStack.size()), index);
            return static_cast<size_t>(index);
        }

        bool IsFalse(int index) const noexcept
        {
            auto absIndex = ToAbsIndex(index);
            assert(absIndex < m_uStackPointer);
            auto& value = m_stStack[absIndex];
            return value.index() == 0 || (value.index() == 1 && std::get<bool>(value) == false) ||
                (value.index() == 2 && std::get<int64_t>(value) == 0);
        }

        Result<void> CastAsNumber(int index) noexcept
        {
            auto absIndex = ToAbsIndex(index);
            assert(absIndex < m_uStackPointer);
            auto& value = m_stStack[absIndex];
            if (value.index() == 0)
            {
                value = static_cast<int64_t>(0);
            }
            else if (value.index() == 1)
            {
                value = static_cast<int64_t>(std::get<bool>(value) ? 1 : 0);
            }
            else if (value.index() == 4)
            {
                char* end;
                auto& str = std::get<std::string>(value);

                // 尝试用整数解析
                auto ll = ::strtoll(str.c_str(), &end, 10);
                if (end == str.c_str() + str.size())
                {
                    value = ll;
                }
                else
                {
                    auto d = ::strtod(str.c_str(), &end);
                    if (end != str.c_str() + str.size())
                        return make_error_code(ExpressionEvaluator::RuntimeError::BadCast);
                    value = d;
                }
            }
            assert(value.index() == 2 || value.index() == 3);
            return {};
        }

        Span<const ExpressionEvaluator::Value> ValueSlice(size_t begin, size_t end) const noexcept
        {
            assert(begin <= m_uStackPointer);
            assert(end <= m_uStackPointer);
            assert(begin <= end);
            return { m_stStack.data() + begin, end - begin };
        }

    private:
        size_t m_uStackPointer = 0;
        std::vector<ExpressionEvaluator::Value> m_stStack;
    };

    namespace VMOperators
    {
        template <template <class> class RealOp>
        struct NonErrorBinaryOp
        {
            template <typename T>
            struct Mixin
            {
                auto operator()(const T& lhs, const T& rhs) const noexcept ->
                    Result<decltype(RealOp<T>{}(std::declval<T>(), std::declval<T>()))>
                {
                    return RealOp<T>{}(lhs, rhs);
                }
            };
        };

        template <typename T>
        struct DivOp
        {
            Result<T> operator()(const T& lhs, const T& rhs) const noexcept
            {
                if constexpr (std::is_integral_v<T>)
                {
                    if (rhs == 0)
                        return make_error_code(ExpressionEvaluator::RuntimeError::DivisionByZero);
                }
                return lhs / rhs;
            }
        };

        template <typename T>
        struct ModOp
        {
            Result<T> operator()(const T& lhs, const T& rhs) const noexcept
            {
                if constexpr (std::is_integral_v<T>)
                {
                    if (rhs == 0)
                        return make_error_code(ExpressionEvaluator::RuntimeError::DivisionByZero);
                    return lhs % rhs;
                }
                else
                {
                    return ::fmod(lhs, rhs);
                }
            }
        };

        struct LShiftOp
        {
            Result<int64_t> operator()(int64_t lhs, int64_t rhs) const noexcept
            {
                return lhs << rhs;
            }
        };

        struct RShiftOp
        {
            Result<int64_t> operator()(int64_t lhs, int64_t rhs) const noexcept
            {
                return lhs >> rhs;
            }
        };

        struct BitAndOp
        {
            Result<int64_t> operator()(int64_t lhs, int64_t rhs) const noexcept
            {
                return lhs & rhs;
            }
        };

        struct BitXorOp
        {
            Result<int64_t> operator()(int64_t lhs, int64_t rhs) const noexcept
            {
                return lhs ^ rhs;
            }
        };

        struct BitOrOp
        {
            Result<int64_t> operator()(int64_t lhs, int64_t rhs) const noexcept
            {
                return lhs | rhs;
            }
        };

        template <template<class> class BasicOp>
        struct NumericStackOp
        {
            Result<void> operator()(ValueStack& stack) const noexcept
            {
                auto& lhs = stack[-2];
                auto& rhs = stack[-1];

                if (lhs.index() == 2)  // int64
                {
                    if (rhs.index() == 2)  // int64
                    {
                        auto result = BasicOp<int64_t>{}(std::get<int64_t>(lhs), std::get<int64_t>(rhs));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(*result);
                    }
                    else if (rhs.index() == 3)  // double
                    {
                        auto result = BasicOp<double>{}(static_cast<double>(std::get<int64_t>(lhs)), std::get<double>(rhs));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(*result);
                    }
                    else
                    {
                        return make_error_code(ExpressionEvaluator::RuntimeError::InvalidArithmeticType);
                    }
                }
                else if (lhs.index() == 3)  // double
                {
                    if (rhs.index() == 2)  // int64
                    {
                        auto result = BasicOp<double>{}(std::get<double>(lhs), static_cast<double>(std::get<int64_t>(rhs)));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(*result);
                    }
                    else if (rhs.index() == 3)  // double
                    {
                        auto result = BasicOp<double>{}(std::get<double>(lhs), std::get<double>(rhs));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(*result);
                    }
                    else
                    {
                        return make_error_code(ExpressionEvaluator::RuntimeError::InvalidArithmeticType);
                    }
                }
                else
                {
                    return make_error_code(ExpressionEvaluator::RuntimeError::InvalidArithmeticType);
                }
                return {};
            }
        };

        template <class BasicIntOp>
        struct IntegerStackOp
        {
            Result<void> operator()(ValueStack& stack) const noexcept
            {
                auto& lhs = stack[-2];
                auto& rhs = stack[-1];

                if (lhs.index() == 2)  // int64
                {
                    if (rhs.index() == 2)  // int64
                    {
                        auto result = BasicIntOp{}(std::get<int64_t>(lhs), std::get<int64_t>(rhs));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(*result);
                    }
                    else if (rhs.index() == 3)  // double
                    {
                        auto result = BasicIntOp{}(std::get<int64_t>(lhs), static_cast<int64_t>(std::get<double>(rhs)));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(*result);
                    }
                    else
                    {
                        return make_error_code(ExpressionEvaluator::RuntimeError::InvalidArithmeticType);
                    }
                }
                else if (lhs.index() == 3)  // double
                {
                    if (rhs.index() == 2)  // int64
                    {
                        auto result = BasicIntOp{}(static_cast<int64_t>(std::get<double>(lhs)), std::get<int64_t>(rhs));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(*result);
                    }
                    else if (rhs.index() == 3)  // double
                    {
                        auto result = BasicIntOp{}(static_cast<int64_t>(std::get<double>(lhs)),
                            static_cast<int64_t>(std::get<double>(rhs)));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(*result);
                    }
                    else
                    {
                        return make_error_code(ExpressionEvaluator::RuntimeError::InvalidArithmeticType);
                    }
                }
                else
                {
                    return make_error_code(ExpressionEvaluator::RuntimeError::InvalidArithmeticType);
                }
                return {};
            }
        };

        template <template<class> class BasicOp>
        struct RelationShipStackOp
        {
            Result<void> operator()(ValueStack& stack) const noexcept
            {
                auto& lhs = stack[-2];
                auto& rhs = stack[-1];

                if (lhs.index() == 2)  // int64
                {
                    if (rhs.index() == 2)  // int64
                    {
                        auto result = BasicOp<int64_t>{}(std::get<int64_t>(lhs), std::get<int64_t>(rhs));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(result);
                    }
                    else if (rhs.index() == 3)  // double
                    {
                        auto result = BasicOp<double>{}(static_cast<double>(std::get<int64_t>(lhs)), std::get<double>(rhs));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(result);
                    }
                    else
                    {
                        return make_error_code(ExpressionEvaluator::RuntimeError::InvalidArithmeticType);
                    }
                }
                else if (lhs.index() == 3)  // double
                {
                    if (rhs.index() == 2)  // int64
                    {
                        auto result = BasicOp<double>{}(std::get<double>(lhs), static_cast<double>(std::get<int64_t>(rhs)));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(result);
                    }
                    else if (rhs.index() == 3)  // double
                    {
                        auto result = BasicOp<double>{}(std::get<double>(lhs), std::get<double>(rhs));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(result);
                    }
                    else
                    {
                        return make_error_code(ExpressionEvaluator::RuntimeError::InvalidArithmeticType);
                    }
                }
                else if (lhs.index() == 4)  // std::string
                {
                    if (rhs.index() == 4)  // std::string
                    {
                        auto result = BasicOp<std::string>{}(std::get<std::string>(lhs), std::get<std::string>(rhs));
                        if (!result)
                            return result.GetError();
                        stack.Pop(2);
                        stack.Push(result);
                    }
                    else
                    {
                        return make_error_code(ExpressionEvaluator::RuntimeError::InvalidArithmeticType);
                    }
                }
                else
                {
                    return make_error_code(ExpressionEvaluator::RuntimeError::InvalidArithmeticType);
                }
                return {};
            }
        };

        struct EqualStackOp
        {
            Result<void> operator()(ValueStack& stack) const noexcept
            {
                auto& lhs = stack[-2];
                auto& rhs = stack[-1];
                bool result = false;

                if (lhs.index() == rhs.index())  // 类型一致，对数据进行判定
                {
                    switch (lhs.index())
                    {
                        case 0:  // nullptr
                            result = true;
                            break;
                        case 1:  // bool
                            result = std::get<bool>(lhs) == std::get<bool>(rhs);
                            break;
                        case 2:  // int64
                            result = std::get<int64_t>(lhs) == std::get<int64_t>(rhs);
                            break;
                        case 3:  // double
                            result = std::get<double>(lhs) == std::get<double>(rhs);
                            break;
                        case 4:  // string
                            result = std::get<std::string>(lhs) == std::get<std::string>(rhs);
                            break;
                        default:
                            assert(false);
                            break;
                    }
                }
                else
                {
                    // 针对数值类型进行提升
                    if (lhs.index() == 2)  // int64
                    {
                        if (rhs.index() == 3)  // double
                        {
                            result = static_cast<double>(std::get<int64_t>(lhs)) == std::get<double>(rhs);
                        }
                        else
                        {
                            result = false;
                        }
                    }
                    else if (lhs.index() == 3)  // double
                    {
                        if (rhs.index() == 2)  // int64
                        {
                            result = std::get<double>(lhs) == static_cast<double>(std::get<int64_t>(rhs));
                        }
                        else
                        {
                            result = false;
                        }
                    }
                    else
                    {
                        result = false;
                    }
                }

                stack.Pop(2);
                stack.Push(result);
                return {};
            }
        };
    }
}

// <editor-fold desc="ExpressionEvaluator::ParseErrorCategory">

const ExpressionEvaluator::CompileErrorCategory& ExpressionEvaluator::CompileErrorCategory::GetInstance() noexcept
{
    static const ExpressionEvaluator::CompileErrorCategory kInstance;
    return kInstance;
}

const char* ExpressionEvaluator::CompileErrorCategory::name() const noexcept
{
    return "CompileError";
}

std::string ExpressionEvaluator::CompileErrorCategory::message(int ev) const
{
    switch (static_cast<CompileError>(ev))
    {
        case CompileError::UnexpectedToken:
            return "unexpected token";
        case CompileError::OpCodeOutOfIndex:
            return "opcode out of range";
        default:
            return "<unknown>";
    }
}

// </editor-fold>
// <editor-fold desc="ExpressionEvaluator::RuntimeErrorCategory">

const ExpressionEvaluator::RuntimeErrorCategory& ExpressionEvaluator::RuntimeErrorCategory::GetInstance() noexcept
{
    static const ExpressionEvaluator::RuntimeErrorCategory kInstance;
    return kInstance;
}

const char* ExpressionEvaluator::RuntimeErrorCategory::name() const noexcept
{
    return "RuntimeError";
}

std::string ExpressionEvaluator::RuntimeErrorCategory::message(int ev) const
{
    switch (static_cast<RuntimeError>(ev))
    {
        case RuntimeError::StackOverflow:
            return "stack overflow";
        case RuntimeError::BadCast:
            return "bad cast";
        case RuntimeError::InvalidArithmeticType:
            return "invalid arithmetic type";
        case RuntimeError::DivisionByZero:
            return "division by zero";
        default:
            return "<unknown>";
    }
}

// </editor-fold>
// <editor-fold desc="ExpressionEvaluator">

Result<ExpressionEvaluator> ExpressionEvaluator::Compile(std::string_view expression) noexcept
{
    ExpressionParser parser(expression);
    auto ret = parser.ParseExpression();
    if (!ret)
        return ret.GetError();

    return ExpressionEvaluator { std::move(parser.GetBytecodes()), std::move(parser.GetConstants()) };
}

ExpressionEvaluator::ExpressionEvaluator(std::vector<uint16_t> bytecodes, std::vector<Value> constants) noexcept
    : m_stBytecodes(std::move(bytecodes)), m_stConstants(std::move(constants))
{
}

Result<ExpressionEvaluator::Value> ExpressionEvaluator::Evaluate(IRuntime* runtime, size_t stackSize) const noexcept
{
    Result<ValueStack> stack = ValueStack::Create(stackSize);
    if (!stack)
        return stack.GetError();

    size_t ip = 0;
    Result<void> ret;
    while (ip < m_stBytecodes.size())
    {
        auto op = (m_stBytecodes[ip] >> 8u) & 0xFFu;
        auto arg = m_stBytecodes[ip] & 0xFFu;
        ++ip;

        switch (static_cast<OpCodes>(op))
        {
            case OpCodes::Nop:
                break;
            case OpCodes::PushNull:
                ret = stack->Push(std::nullptr_t {});
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::PushTrue:
                ret = stack->Push(true);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::PushFalse:
                ret = stack->Push(false);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::Jump:
                ip = ip + arg - 1;
                break;
            case OpCodes::JumpIfFalse:
                if (stack->IsFalse(-1))
                    ip = ip + arg - 1;
                stack->Pop();
                break;
            case OpCodes::JumpIfTrue:
                if (!stack->IsFalse(-1))
                    ip = ip + arg - 1;
                stack->Pop();
                break;
            case OpCodes::Positive:
                ret = stack->CastAsNumber(-1);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::Negative:
                ret = stack->CastAsNumber(-1);
                if (!ret)
                {
                    return ret.GetError();
                }
                else
                {
                    assert(stack->GetStackPointer() > 0);
                    auto& top = stack->Top();
                    if (top.index() == 2)
                    {
                        top = -std::get<int64_t>(top);
                    }
                    else
                    {
                        assert(top.index() == 3);
                        top = -std::get<double>(top);
                    }
                    assert(top.index() == 2 || top.index() == 3);
                }
                break;
            case OpCodes::BitNot:
                ret = stack->CastAsNumber(-1);
                if (!ret)
                {
                    return ret.GetError();
                }
                else
                {
                    auto& top = stack->Top();
                    if (top.index() == 2)
                    {
                        top = ~std::get<int64_t>(top);
                    }
                    else
                    {
                        assert(top.index() == 3);
                        top = ~static_cast<int64_t>(std::get<double>(top));
                    }
                    assert(top.index() == 2);
                }
                break;
            case OpCodes::LogicalNot:
                {
                    auto& top = stack->Top();
                    top = !!stack->IsFalse(-1);
                }
                break;
            case OpCodes::Mul:
                ret = VMOperators::NumericStackOp<VMOperators::NonErrorBinaryOp<std::multiplies>::Mixin>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::Div:
                ret = VMOperators::NumericStackOp<VMOperators::DivOp>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::Mod:
                ret = VMOperators::NumericStackOp<VMOperators::ModOp>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::Add:
                ret = VMOperators::NumericStackOp<VMOperators::NonErrorBinaryOp<std::plus>::Mixin>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::Sub:
                ret = VMOperators::NumericStackOp<VMOperators::NonErrorBinaryOp<std::minus>::Mixin>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::LShift:
                ret = VMOperators::IntegerStackOp<VMOperators::LShiftOp>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::RShift:
                ret = VMOperators::IntegerStackOp<VMOperators::RShiftOp>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::BitAnd:
                ret = VMOperators::IntegerStackOp<VMOperators::BitAndOp>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::BitXor:
                ret = VMOperators::IntegerStackOp<VMOperators::BitXorOp>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::BitOr:
                ret = VMOperators::IntegerStackOp<VMOperators::BitOrOp>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::Less:
                ret = VMOperators::RelationShipStackOp<VMOperators::NonErrorBinaryOp<std::less>::Mixin>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::Greater:
                ret = VMOperators::RelationShipStackOp<VMOperators::NonErrorBinaryOp<std::greater>::Mixin>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::LessEq:
                ret = VMOperators::RelationShipStackOp<VMOperators::NonErrorBinaryOp<std::less_equal>::Mixin>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::GreaterEq:
                ret = VMOperators::RelationShipStackOp<VMOperators::NonErrorBinaryOp<std::greater_equal>::Mixin>{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::Equal:
                ret = VMOperators::EqualStackOp{}(*stack);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::NotEqual:
                ret = VMOperators::EqualStackOp{}(*stack);
                if (!ret)
                    return ret.GetError();
                assert(stack->Top().index() == 1);
                stack->Top() = !std::get<bool>(stack->Top());
                break;
            case OpCodes::LoadConstant:
                assert(arg < m_stConstants.size());
                ret = stack->Push(m_stConstants[arg]);
                if (!ret)
                    return ret.GetError();
                break;
            case OpCodes::LoadValue:
                {
                    assert(arg < m_stConstants.size());
                    auto& constant = m_stConstants[arg];
                    assert(constant.index() == 4);
                    if (!runtime)
                    {
                        ret = stack->Push(std::nullptr_t{});
                    }
                    else
                    {
                        auto value = runtime->GetVariable(std::get<std::string>(constant));
                        if (!value)
                            return value.GetError();
                        ret = stack->Push(std::move(*value));
                    }
                    if (!ret)
                        return ret.GetError();
                }
                break;
            case OpCodes::CallFunction:
                if (!runtime)
                {
                    stack->Pop(static_cast<int>(arg + 1));
                    stack->Push(std::nullptr_t{});
                }
                else
                {
                    auto& func = stack->operator[](-static_cast<int>(arg + 1));
                    assert(func.index() == 4);
                    auto begin = stack->ToAbsIndex(-static_cast<int>(arg));
                    auto end = stack->ToAbsIndex(-1) + 1;

                    auto value = runtime->CallFunction(std::get<std::string>(func), stack->ValueSlice(begin, end));
                    if (!value)
                        return value.GetError();

                    stack->Pop(static_cast<int>(arg + 1));
                    stack->Push(std::move(*value));
                }
                break;
            default:
                break;
        }
    }
    assert(ip == m_stBytecodes.size());
    assert(stack->GetStackPointer() == 1);
    return stack->Top();
}

// </editor-fold>
