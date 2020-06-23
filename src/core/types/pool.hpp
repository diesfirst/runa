#ifndef TYPES_POOL_H_
#define TYPES_POOL_H_ 

#include <memory>
#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>
#include <render/command.hpp>
//#include <concepts>

namespace sword
{

namespace state { class Report; }

template <typename T, typename Base, size_t Size>
class Pool
{
private:
    std::unique_ptr<render::CommandPool_t<Size>> gpuCommandPool;
    std::array<T, Size> pool;

public:
    //using Pointer = std::unique_ptr<Base, std::function<void(Base*)>>;
    using Pointer = std::unique_ptr<Base, std::function<void(Base*)>>;

    Pool()
    {}
    Pool(render::CommandPool_t<Size>&& gpuPool)
    {
        constexpr bool hasCmdBufferMember = requires (T t, render::CommandBuffer& buffer)
        {
//            t.commandBuffer;
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
    Pointer request(Args... args)
    {
        std::cout << "Called first pool" << '\n';
        for (int i = 0; i < Size; i++) 
            if (pool[i].isAvailable())
            {
                pool[i].set(args...);
                pool[i].activate();
                Pointer ptr{&pool[i], [](Base* t){ t->reset(); }}; //may not have to construct here
                return ptr; //tentatively may need to be std::move? copy should be ellided tho
            }    
        return nullptr;
    }

    template <typename... Args> 
    Pointer request(std::function<void(state::Report*)> reportCb, Args... args) //requires IsCommand<T>
    {
        std::cout << "Called second pool request" << std::endl;
        for (int i = 0; i < Size; i++) 
            if (pool[i].isAvailable())
            {
                pool[i].setSuccessFn(reportCb);
                pool[i].set(args...);
                pool[i].activate();
                Pointer ptr{&pool[i], [](Base* t){ t->reset(); }};
                return ptr; //tentatively may need to be std::move? copy should be ellided tho
            }    
        return nullptr;
    }

    void printAll() const 
    {
        for (auto& i : pool) 
            i.print();
    }

};


};

#endif /* ifndef POOL_H_ */
