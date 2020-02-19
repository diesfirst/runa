#include <iostream>
#include <tuple>
#include <vector>
#include <memory>
#include <functional>
#include "../src/core/util.hpp"

namespace IS
{
template <int... Ns> 
struct sequence {};

template <int... Ns>
struct seq_gen;

template <int I, int... Ns>
struct seq_gen<I, Ns...>
{
    using type = typename seq_gen<I - 1, I - 1, Ns...>::type;
};

template <int... Ns>
struct seq_gen<0, Ns...>
{
    using type = sequence<Ns...>;
};

template <int N>
using sequence_t = typename seq_gen<N>::type;
};

namespace tc
{

static void func(double d, float f, int i)
{
    std::cout << d << ", "
        << f << ", "
        << i << std::endl;
}

template <typename F, typename TUP, int... Indices>
static void tuplecall(F f, TUP tup, IS::sequence<Indices...>)
{
    f(std::get<Indices>(tup)...);
};

template <typename F, typename... Ts>
static void tuplecall(F f, std::tuple<Ts...> tup)
{
    tuplecall(f, tup, IS::sequence_t<sizeof...(Ts)>{});
}
};

class Poolable
{
public:
    virtual void set()
    {
        std::cout << "Base set called" << std::endl;
        inUse = true;
    }
    inline bool isAvailable() { return !inUse;}
    virtual void reset() {inUse = false;}
protected:
    Poolable() = default;
    bool inUse{false};
};

class Beast : public Poolable
{
public:
    Beast() {reset();}
    void reset() {name = "default"; x = 0; y = 0; inUse = false;}
    inline void set(std::string name, int x, int y) {Poolable::set(); this->name = name; this->x = x; this->y = y;}
    void print() const 
    {
        std::cout << name 
            << " x: " << x
            << " y: " << y
            << " in use: " << inUse
            << std::endl;
    }
private:
    std::string name;
    int x, y;
};

class Simple : public Poolable
{
public:
    void print() const
    {
        std::cout << name << " in use: " << inUse << std::endl;
    }
private:
    std::string name = "foobar";
};

template <typename T>
class Pool
{
public:
    template <typename P> using Pointer = std::unique_ptr<P, std::function<void(P*)>>;

    Pool(size_t size) : size{size}, pool(size) {}

    template <typename... Args> Pointer<Poolable> request(Args... args)
    {
        for (int i = 0; i < size; i++) 
        {
            if (pool[i].isAvailable())
            {
                std::cout << "yes" << std::endl;
                pool[i].set(args...);
                Pointer<Poolable> ptr{&pool[i], [](Poolable* t)
                    {
                        t->reset();
                    }};
                return std::move(ptr);
            }    
        }
        return nullptr;
    }

    auto getPoolCount() {return pool.size();}
    void printAll() const 
    {
        for (auto& i : pool) 
            i.print();
    }

private:

    const size_t size;
    std::vector<T> pool;
};

Timer timer;

int main()
{
    Pool<Beast> beastpool{3};
    std::vector<std::unique_ptr<Poolable, std::function<void(Poolable*)>>> poolables;
    beastpool.printAll();
    {
        auto beast = beastpool.request("joe", 5, 2);
        auto beast2 = beastpool.request("tim", 2, 1);
        poolables.push_back(std::move(beast));
        beastpool.printAll();
    }
    beastpool.printAll();

    std::cout << "making simple pool" << std::endl;
    Pool<Simple> simplepool{3};
    simplepool.printAll();
    {
        std::cout << "in scope" << std::endl;
        auto p1 = simplepool.request();
        auto p2 = simplepool.request();
        poolables.push_back(std::move(p1));
        simplepool.printAll();
    }
    std::cout << "out of scope" << std::endl;
    simplepool.printAll();
    poolables.pop_back();
    poolables.pop_back();
    beastpool.printAll();
    simplepool.printAll();
}
