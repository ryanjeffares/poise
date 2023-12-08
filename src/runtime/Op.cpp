#include "Op.hpp"

namespace fmt {
using namespace poise::runtime;

auto formatter<Op>::format(poise::runtime::Op op, fmt::format_context& context) -> decltype(context.out())
{
    string_view result = "unknown";

    switch (op) {
        case Op::AssignLocal:
            result = "AssignLocal";
            break;
        case Op::CaptureLocal:
            result = "CaptureLocal";
            break;
        case Op::ConstructBuiltin:
            result = "ConstructBuiltin";
            break;
        case Op::DeclareFunction:
            result = "DeclareFunction";
            break;
        case Op::DeclareLocal:
            result = "DeclareLocal";
            break;
        case Op::EnterTry:
            result = "EnterTry";
            break;
        case Op::ExitTry:
            result = "ExitTry";
            break;
        case Op::LoadCapture:
            result = "LoadCapture";
            break;
        case Op::LoadConstant:
            result = "LoadConstant";
            break;
        case Op::LoadFunction:
            result = "LoadFunction";
            break;
        case Op::LoadLocal:
            result = "LoadLocal";
            break;
        case Op::LoadType:
            result = "LoadType";
            break;
        case Op::Pop:
            result = "Pop";
            break;
        case Op::PopLocals:
            result = "PopLocals";
            break;
        case Op::TypeOf:
            result = "TypeOf";
            break;
        case Op::Print:
            result = "Print";
            break;
        case Op::LogicOr:
            result = "LogicOr";
            break;
        case Op::LogicAnd:
            result = "LogicAnd";
            break;
        case Op::BitwiseOr:
            result = "BitwiseOr";
            break;
        case Op::BitwiseXor:
            result = "BitwiseXor";
            break;
        case Op::BitwiseAnd:
            result = "BitwiseAnd";
            break;
        case Op::Equal:
            result = "Equal";
            break;
        case Op::NotEqual:
            result = "NotEqual";
            break;
        case Op::LessThan:
            result = "LessThan";
            break;
        case Op::LessEqual:
            result = "LessEqual";
            break;
        case Op::GreaterThan:
            result = "GreaterThan";
            break;
        case Op::GreaterEqual:
            result = "GreaterEqual";
            break;
        case Op::LeftShift:
            result = "LeftShift";
            break;
        case Op::RightShift:
            result = "RightShift";
            break;
        case Op::Addition:
            result = "Addition";
            break;
        case Op::Subtraction:
            result = "Subtraction";
            break;
        case Op::Multiply:
            result = "Multiply";
            break;
        case Op::Divide:
            result = "Divide";
            break;
        case Op::Modulus:
            result = "Modulus";
            break;
        case Op::LogicNot:
            result = "LogicNot";
            break;
        case Op::BitwiseNot:
            result = "BitwiseNot";
            break;
        case Op::Negate:
            result = "Negate";
            break;
        case Op::Plus:
            result = "Plus";
            break;
        case Op::Call:
            result = "Call";
            break;
        case Op::CallNative:
            result = "CallNative";
            break;
        case Op::Jump:
            result = "Jump";
            break;
        case Op::JumpIfFalse:
            result = "JumpIfFalse";
            break;
        case Op::JumpIfTrue:
            result = "JumpIfTrue";
            break;
        case Op::Exit:
            result = "Exit";
            break;
        case Op::Return:
            result = "Return";
            break;
    }

    return formatter<string_view>::format(result, context);
}
}   // namespace fmt
