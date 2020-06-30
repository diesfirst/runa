#ifndef EVENT_QUEUE_HPP
#define EVENT_QUEUE_HPP

#include "event/types.hpp"
namespace sword
{

namespace event
{

class EventQueue
{
public:
    void push(EventPtr&& eventPtr) { queue[curSize] = eventPtr; }
private:
    int curSize{0};
    static constexpr int maxSize = 50;
    std::array<EventPtr, maxSize> queue;
};

}; // namespace event

}; // namespace sword

#endif /* end of include guard: EVENT_QUEUE_HPP */
