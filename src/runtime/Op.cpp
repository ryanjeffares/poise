#include "Op.hpp"

using namespace poise::runtime;

auto fmt::formatter<Op>::format(Op op, format_context& context) const -> decltype(context.out())
{
    switch (op) {
        case Op::AssignLocal:
            return formatter<string_view>::format("AssignLocal", context);
        case Op::CaptureLocal:
            return formatter<string_view>::format("CaptureLocal", context);
        case Op::ConstructBuiltin:
            return formatter<string_view>::format("ConstructBuiltin", context);
        case Op::DeclareLocal:
            return formatter<string_view>::format("DeclareLocal", context);
        case Op::DeclareLocalsWithUnpack:
            return formatter<string_view>::format("DeclareLocalsWithUnpack", context);
        case Op::EnterTry:
            return formatter<string_view>::format("EnterTry", context);
        case Op::ExitTry:
            return formatter<string_view>::format("ExitTry", context);
        case Op::LoadCapture:
            return formatter<string_view>::format("LoadCapture", context);
        case Op::LoadConstant:
            return formatter<string_view>::format("LoadConstant", context);
        case Op::LoadFunctionOrStruct:
            return formatter<string_view>::format("LoadFunction", context);
        case Op::LoadLocal:
            return formatter<string_view>::format("LoadLocal", context);
        case Op::LoadMember:
            return formatter<string_view>::format("LoadMember", context);
        case Op::LoadType:
            return formatter<string_view>::format("LoadType", context);
        case Op::Pop:
            return formatter<string_view>::format("Pop", context);
        case Op::PopIterator:
            return formatter<string_view>::format("PopIterator", context);
        case Op::PopLocals:
            return formatter<string_view>::format("PopLocals", context);
        case Op::Throw:
            return formatter<string_view>::format("Throw", context);
        case Op::Unpack:
            return formatter<string_view>::format("Unpack", context);
        case Op::Assert:
            return formatter<string_view>::format("Assert", context);
        case Op::TypeOf:
            return formatter<string_view>::format("TypeOf", context);
        case Op::Print:
            return formatter<string_view>::format("Print", context);
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
        case Op::MakeLambda:
            return formatter<string_view>::format("MakeLambda", context);
        case Op::AssignIndex:
            return formatter<string_view>::format("AssignIndex", context);
        case Op::LoadIndex:
            return formatter<string_view>::format("LoadIndex", context);
        case Op::Call:
            return formatter<string_view>::format("Call", context);
        case Op::CallNative:
            return formatter<string_view>::format("CallNative", context);
        case Op::IncrementIterator:
            return formatter<string_view>::format("IncrementIterator", context);
        case Op::InitIterator:
            return formatter<string_view>::format("InitIterator", context);
        case Op::Jump:
            return formatter<string_view>::format("Jump", context);
        case Op::JumpIfFalse:
            return formatter<string_view>::format("JumpIfFalse", context);
        case Op::JumpIfTrue:
            return formatter<string_view>::format("JumpIfTrue", context);
        case Op::Exit:
            return formatter<string_view>::format("Exit", context);
        case Op::Return:
            return formatter<string_view>::format("Return", context);
        default:
            POISE_UNREACHABLE();
    }
}
