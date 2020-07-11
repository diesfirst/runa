#ifndef EVENT_TYPES_HPP
#define EVENT_TYPES_HPP

#include <memory>
#include <functional>
#include <types/stack.hpp>
#include <types/pool.hpp>
#include <types/queue.hpp>

namespace sword
{

namespace event
{

class Event;

template <typename T, size_t N>
using Pool = container::Pool<T, Event, N>;

using Vessel = container::PoolVessel<Event>;

using EventQueue = container::LockingQueue<Vessel, 200>;

}; // namespace event

}; // namespace sword

#endif /* end of include guard: EVENT_TYPES_HPP */
