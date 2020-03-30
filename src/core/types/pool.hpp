#ifndef TYPES_POOL_H_
#define TYPES_POOL_H_ 

#include <memory>
#include <algorithm>
#include <functional>
#include <vector>

namespace sword
{

constexpr uint32_t POOL_DEFAULT_SIZE = 3;

template <typename T, typename Base>
class Pool
{
public:
    template <typename P> using Pointer = std::unique_ptr<P, std::function<void(P*)>>;

    Pool(size_t size) : size{size}, pool(size) {}
    Pool() : size{POOL_DEFAULT_SIZE}, pool(size) {}

    template <typename... Args> Pointer<Base> request(Args... args)
    {
        for (int i = 0; i < size; i++) 
            if (pool[i].isAvailable())
            {
                pool[i].set(args...);
                pool[i].activate();
                Pointer<Base> ptr{&pool[i], [](Base* t)
                    {
                        t->reset();
                    }};
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

    template <typename... Args> void initialize(Args&&... args)
    {
        for (int i = 0; i < size; i++) 
            pool.emplace_back(args...);   
    }

    const size_t size;
    std::vector<T> pool;
};

};

#endif /* ifndef POOL_H_ */
