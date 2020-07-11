#ifndef TYPES_POOL_H_
#define TYPES_POOL_H_ 

#include <memory>
#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>
#include <render/command.hpp>
#include <util/debug.hpp>
//#include <concepts>

namespace sword
{

namespace state { class Report; }

namespace container
{

//template<typename T>
//concept Resetable = requires(T t)
//{
//    t.reset();
//};
//
//template<Resetable T>
//class PoolVessel
//...
// clang still gets this wrong. gcc is fine with it, but im sick of seeing the 
// error diagnostics

template<typename T>
class PoolVessel
{
public:
    PoolVessel() = default;
    PoolVessel(T* pT) { handle = pT; }
    ~PoolVessel() { if (handle) handle->reset(); }
    PoolVessel(const PoolVessel&) = delete;
    PoolVessel& operator=(const PoolVessel&) = delete;
    PoolVessel& operator=(PoolVessel&& other) 
    { 
        if (this == &other) return *this;
        handle = other.handle; other.handle = nullptr; return *this;
    }
    PoolVessel(PoolVessel&& other) : handle{other.handle} {other.handle = nullptr;}
    T* operator->() { return handle; }
    T* get() { return handle; }
    operator bool() const { return handle != nullptr; }
private:
    T* handle = nullptr;
};

template <typename T, typename Base, size_t Size>
class Pool
{
private:
    std::unique_ptr<render::CommandPool_t<Size>> gpuCommandPool;
    std::array<T, Size> pool;

public:
    //using Pointer = std::unique_ptr<Base, std::function<void(Base*)>>;
    using Vessel = PoolVessel<Base>;

    Pool() {}

    Pool(render::CommandPool_t<Size>&& gpuPool)
    {
        constexpr bool hasCmdBufferMember = requires (T t, render::CommandBuffer& buffer)
        {
            t.setCommandBuffer(buffer);
        };
        static_assert (hasCmdBufferMember);
        if constexpr(hasCmdBufferMember)
        {
            std::cout << "Constructing gpuCommandPool" << '\n';
            gpuCommandPool = std::make_unique<render::CommandPool_t<Size>>(std::move(gpuPool));
            for (int i = 0; i < Size; i++) 
            {
                pool[i].setCommandBuffer(gpuCommandPool->requestCommandBuffer(i));
            }
        }
    }

    template <typename... Args> 
    Vessel request(Args... args)
    {
        for (int i = 0; i < Size; i++) 
            if (pool[i].isAvailable())
            {
                pool[i].set(args...);
                pool[i].activate();
                Vessel vessel{&pool[i]};
                return vessel; //tentatively may need to be std::move? copy should be ellided tho
            }    
        SWD_DEBUG_MSG("No more room. Size: " << Size);
        throw std::runtime_error("Ran out of room in pool. ");
    }

    template <typename... Args> 
    Vessel request(std::function<void(state::Report*)> reportCb, Args... args) //requires IsCommand<T>
    {
        for (int i = 0; i < Size; i++) 
            if (pool[i].isAvailable())
            {
                pool[i].setSuccessFn(reportCb);
                pool[i].set(args...);
                pool[i].activate();
                Vessel vessel{&pool[i]};
                return vessel; //tentatively may need to be std::move? copy should be ellided tho
            }    
        SWD_DEBUG_MSG("No more room. Size: " << Size);
        throw std::runtime_error("Ran out of room in pool. ");
    }

    static void printAll(Pool* pool) 
    {
        for (auto& i : pool->pool) 
            i.print();
    }

};


} // namespace container



}

#endif /* ifndef POOL_H_ */
