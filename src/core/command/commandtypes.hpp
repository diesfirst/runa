#include <command/command.hpp>
#include <memory>
#include <functional>
#include <types/stack.hpp>
#include <types/pool.hpp>

using CmdPtr = std::unique_ptr<sword::command::Command, std::function<void(sword::command::Command*)>>;
using CommandStack = sword::ForwardStack<CmdPtr>;

template <typename T>
using CommandPool = sword::Pool<T, sword::command::Command>;

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
