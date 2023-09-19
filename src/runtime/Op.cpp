#include "Op.hpp"

namespace fmt
{
    using namespace poise::runtime;

    auto formatter<Op>::format(poise::runtime::Op op, fmt::format_context& context) -> decltype(context.out())
    {
        switch (op) {
            case Op::Call:
                return formatter<string_view>::format("Call", context);
            case Op::DeclareFunction:
                return formatter<string_view>::format("DeclareFunction", context);
            case Op::Exit:
                return formatter<string_view>::format("Exit", context);
            case Op::LoadConstant:
                return formatter<string_view>::format("LoadConstant", context);
            case Op::PrintLn:
                return formatter<string_view>::format("PrintLn", context);
            case Op::Return:
                return formatter<string_view>::format("Print", context);
        }
    }
}