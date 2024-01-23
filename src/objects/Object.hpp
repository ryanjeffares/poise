#ifndef POISE_OBJECT_HPP
#define POISE_OBJECT_HPP

#include "../Poise.hpp"
#include "../runtime/Types.hpp"

#include <string>

namespace poise::runtime {
class Value;
}

namespace poise::objects {
class Exception;
class Function;
class Type;

namespace iterables {
class Iterable;
class Iterator;
class List;
class Range;
class Tuple;

namespace hashables {
class Dict;
class Hashable;
class Set;
}
}

class Object
{
public:
    Object() = default;

    Object(const Object&) = delete;
    Object(Object&&) = delete;

    virtual ~Object() = default;

    auto incrementRefCount() noexcept -> usize;
    [[nodiscard]] auto decrementRefCount() noexcept -> usize;
    [[nodiscard]] auto refCount() const noexcept -> usize;
    [[nodiscard]] auto tracking() const noexcept -> bool;
    auto setTracking(bool track) noexcept -> void;

    [[nodiscard]] virtual auto asIterable() noexcept -> iterables::Iterable*;
    [[nodiscard]] virtual auto asHashable() noexcept -> iterables::hashables::Hashable*;

    [[nodiscard]] virtual auto asDictionary() noexcept -> iterables::hashables::Dict*;
    [[nodiscard]] virtual auto asException() noexcept -> Exception*;
    [[nodiscard]] virtual auto asFunction() noexcept -> Function*;
    [[nodiscard]] virtual auto asIterator() noexcept -> iterables::Iterator*;
    [[nodiscard]] virtual auto asList() noexcept -> iterables::List*;
    [[nodiscard]] virtual auto asRange() noexcept -> iterables::Range*;
    [[nodiscard]] virtual auto asSet() noexcept -> iterables::hashables::Set*;
    [[nodiscard]] virtual auto asTuple() noexcept -> iterables::Tuple*;
    [[nodiscard]] virtual auto asType() noexcept -> Type*;

    [[nodiscard]] virtual auto toString() const noexcept -> std::string = 0;
    [[nodiscard]] virtual auto type() const noexcept -> runtime::types::Type = 0;
    virtual auto findObjectMembers(std::vector<Object*>& objects) const noexcept -> void = 0;
    virtual auto removeObjectMembers() noexcept -> void = 0;
    [[nodiscard]] virtual auto anyMemberMatchesRecursive(const Object* object) const noexcept -> bool;

    [[nodiscard]] virtual auto iterable() const -> bool;

private:
    usize m_refCount{};
    bool m_tracking{};
};  // class PoiseObjects
}   // namespace poise::objects

#endif  // #ifndef POISE_OBJECT_HPP
