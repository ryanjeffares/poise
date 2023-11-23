#include "Op.hpp"

namespace fmt
{
    using namespace poise::runtime;

    auto formatter<Op>::format(poise::runtime::Op op, fmt::format_context& context) -> decltype(context.out())
    {
        switch (op) {
            case Op::DeclareFunction:
                return formatter<string_view>::format("DeclareFunction", context);
            case Op::LoadConstant:
                return formatter<string_view>::format("LoadConstant", context);
            case Op::PrintLn:
                return formatter<string_view>::format("PrintLn", context);
            case Op::LogicOr:
                return formatter<string_view>::format("LogicOr", context);
            case Op::LogicAnd:
                return formatter<string_view>::format("LogicAnd", context);
            case Op::BitwiseOr:
                return formatter<string_view>::format("BitwiseOr", context);
            case Op::BitwiseXor:
                return formatter<string_view>::format("BitwiseXor", context);
            case Op::BitwiseAnd:
                return formatter<string_view>::format("BitwiseAnd", context);
            case Op::Equal:
                return formatter<string_view>::format("Equal", context);
            case Op::NotEqual:
                return formatter<string_view>::format("NotEqual", context);
            case Op::LessThan:
                return formatter<string_view>::format("LessThan", context);
            case Op::LessEqual:
                return formatter<string_view>::format("LessEqual", context);
            case Op::GreaterThan:
                return formatter<string_view>::format("GreaterThan", context);
            case Op::GreaterEqual:
                return formatter<string_view>::format("GreaterEqual", context);
            case Op::LeftShift:
                return formatter<string_view>::format("LeftShift", context);
            case Op::RightShift:
                return formatter<string_view>::format("RightShift", context);
            case Op::Addition:
                return formatter<string_view>::format("Addition", context);
            case Op::Subtraction:
                return formatter<string_view>::format("Subtraction", context);
            case Op::Multiply:
                return formatter<string_view>::format("Multiply", context);
            case Op::Divide:
                return formatter<string_view>::format("Divide", context);
            case Op::Modulus:
                return formatter<string_view>::format("Modulus", context);
            case Op::LogicNot:
                return formatter<string_view>::format("LogicNot", context);
            case Op::BitwiseNot:
                return formatter<string_view>::format("BitwiseNot", context);
            case Op::Negate:
                return formatter<string_view>::format("Negate", context);
            case Op::Plus:
                return formatter<string_view>::format("Plus", context);
            case Op::Call:
                return formatter<string_view>::format("Call", context);
            case Op::Jump:
                return formatter<string_view>::format("Jump", context);
            case Op::JumpIfFalse:
                return formatter<string_view>::format("JumpIfFalse", context);
            case Op::JumpIfTrue:
                return formatter<string_view>::format("JumpIfTrue", context);
            case Op::Exit:
                return formatter<string_view>::format("Exit", context);
            case Op::Return:
                return formatter<string_view>::format("Print", context);
            default:
                POISE_UNREACHABLE();
                return formatter<string_view>::format("unknown", context);
        }
    }
}
