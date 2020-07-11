#ifndef TYPES_QUEUE_HPP
#define TYPES_QUEUE_HPP

#include <array>
#include <iostream>
#include <mutex>

namespace sword
{

namespace container
{

template <typename T,size_t N>
class LockingQueue
{
public:
    void push(T item) { lock.lock(); items[count] = item; count++; lock.unlock(); }
    void pop() { lock.lock(); count--; lock.unlock(); }
    T& top() {return items[count]; }
    auto topPtr() {return items[count].get();}
    bool empty() const {return count == 0;}
    void clear() { lock.lock(); count = 0; lock.unlock(); }
    void print() const {for (const auto& item : items) std::cout << item->getName() << '\n';}
    size_t size() const {return count; }
    void reverse() { std::reverse(items.begin(), items.begin() + count); }
    T& operator[](int i) { return items[i]; }
private:
    static constexpr size_t maxItems = N; 
    size_t count = 0;
    std::array<T, N> items;
    std::mutex lock;
};

} // namespace sword


} // namespace container

#endif /* end of include guard: TYPES_QUEUE_HPP */
