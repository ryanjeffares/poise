//
// Created by ryand on 13/12/2023.
//

#include "PoiseIterable.hpp"

namespace poise::objects::iterables {

PoiseIterable::PoiseIterable(usize initialSize, const runtime::Value& defaultValue)
    : m_data(initialSize, defaultValue)
{

}

PoiseIterable::PoiseIterable(std::vector<runtime::Value> data)
    : m_data{std::move(data)}
{

}

PoiseIterable::~PoiseIterable()
{
    invalidateIterators();
}

auto PoiseIterable::addIterator(PoiseIterator* iterator) noexcept -> void
{
#ifdef POISE_DEBUG
    const auto it = std::find(m_activeIterators.cbegin(), m_activeIterators.cend(), iterator);
    POISE_ASSERT(it == m_activeIterators.cend(), "Iterator already added");
#endif

    m_activeIterators.push_back(iterator);
}

auto PoiseIterable::removeIterator(PoiseIterator* iterator) noexcept -> void
{
    [[maybe_unused]] const auto cnt = std::erase(m_activeIterators, iterator);
#ifdef POISE_DEBUG
    POISE_ASSERT(cnt > 0, "Trying to remove iterator that was never added");
#endif
}

auto PoiseIterable::data() const noexcept -> std::span<const runtime::Value>
{
    return m_data;
}

auto PoiseIterable::invalidateIterators() noexcept -> void
{
    for (auto iterator : m_activeIterators) {
        iterator->invalidate();
    }
}
}   // namespace poise::objects::iterables