//
// Created by ryand on 13/12/2023.
//

#include "Iterable.hpp"

#include <ranges>

namespace poise::objects::iterables {
Iterable::Iterable(usize initialSize, const runtime::Value& defaultValue)
    : m_data(initialSize, defaultValue)
{

}

Iterable::Iterable(std::vector<runtime::Value> data)
    : m_data{std::move(data)}
{

}

Iterable::~Iterable()
{
    invalidateIterators();
}

auto Iterable::asIterable() noexcept -> Iterable*
{
    return this;
}

auto Iterable::findObjectMembers(std::unordered_set<Object*>& objects) const noexcept -> void
{
    if (type() == runtime::types::Type::Range) {
        return;
    }

    for (const auto& value : m_data) {
        if (const auto object = value.object()) {
            if (auto [it, inserted] = objects.insert(object); inserted) {
                object->findObjectMembers(objects);
            }
        }
    }
}

auto Iterable::removeObjectMembers() noexcept -> void
{
    if (type() == runtime::types::Type::Range) {
        return;
    }

    for (auto& value : m_data) {
        if (value.object() != nullptr) {
            value = runtime::Value::none();
        }
    }
}

auto Iterable::anyMemberMatchesRecursive(const Object* object) const noexcept -> bool
{
    if (type() == runtime::types::Type::Range) {
        return object == this;
    }

    return std::ranges::any_of(m_data, [object, this] (const auto& value) -> bool {
        const auto member = value.object();
        return member != nullptr && (member == this || member == object || member->anyMemberMatchesRecursive(object));
    });
}

auto Iterable::addIterator(Iterator* iterator) noexcept -> void
{
#ifdef POISE_DEBUG
    const auto it = std::ranges::find(m_activeIterators, iterator);
    POISE_ASSERT(it == m_activeIterators.end(), "Iterator already added");
#endif

    m_activeIterators.push_back(iterator);
}

auto Iterable::removeIterator(Iterator* iterator) noexcept -> void
{
    [[maybe_unused]] const auto cnt = std::erase(m_activeIterators, iterator);
#ifdef POISE_DEBUG
    POISE_ASSERT(cnt > 0, "Trying to remove iterator that was never added");
#endif
}

auto Iterable::data() const noexcept -> std::span<const runtime::Value>
{
    return m_data;
}

auto Iterable::invalidateIterators() const noexcept -> void
{
    for (const auto iterator : m_activeIterators) {
        iterator->invalidate();
    }
}
}   // namespace poise::objects::iterables
