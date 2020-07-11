#include <command/command.hpp>
#include <memory>
#include <functional>
#include <types/stack.hpp>
#include <types/pool.hpp>
#include <types/queue.hpp>

namespace sword
{

namespace command
{

using Vessel = container::PoolVessel<Command>;
using Queue = container::LockingQueue<Vessel, 20>;

template <typename T, size_t N>
using Pool = container::Pool<T, Command, N>;

} // namespace command

} // namespace sword

//template <typename T>
//class CommandPool
//{
//public:
//
////    template <typename... Args>
//
//private:
//    sword::Pool<T, sword::command::Command> pool;
//};
