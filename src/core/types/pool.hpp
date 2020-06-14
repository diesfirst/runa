#ifndef TYPES_POOL_H_
#define TYPES_POOL_H_ 

#include <memory>
#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>

namespace sword
{

namespace state { class Report; }

template <typename T, typename Base, size_t Size = 3>
class Pool
{
private:
    std::array<T, Size> pool;
    static constexpr size_t size = Size;
public:

    Pool() {}

    using Pointer = std::unique_ptr<Base, std::function<void(Base*)>>;

    template <typename... Args> Pointer
    request(Args... args)
    {
        std::cout << "Called first pool" << '\n';
        for (int i = 0; i < size; i++) 
            if (pool[i].isAvailable())
            {
                pool[i].set(args...);
                pool[i].activate();
                Pointer ptr{&pool[i], [](Base* t){ t->reset(); }};
                return ptr; //tentatively may need to be std::move? copy should be ellided tho
            }    
        return nullptr;
    }

    template <typename... Args> 
    Pointer request(std::function<void(state::Report*)> reportCb, Args... args)
    {
        std::cout << "Called second pool request" << std::endl;
        for (int i = 0; i < size; i++) 
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

private:

//    template <typename... Args> void initialize(Args&&... args)
//    {
//        for (int i = 0; i < size; i++) 
//            pool.emplace_back(args...);   
//    }

};


};

#endif /* ifndef POOL_H_ */
