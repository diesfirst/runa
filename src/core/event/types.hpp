#ifndef EVENT_TYPES_HPP
#define EVENT_TYPES_HPP

#include <memory>
#include <functional>
#include <types/stack.hpp>
#include <types/pool.hpp>

namespace sword
{

namespace event
{

class Event;

template <typename T, size_t S>
using EventPool = Pool<T, Event, S>;

using EventPtr = std::unique_ptr<Event, std::function<void(Event*)>>;

using EventQueue = ForwardStack<EventPtr>;

}; // namespace event

}; // namespace sword

#endif /* end of include guard: EVENT_TYPES_HPP */
