#include <iostream>
#include <memory>

/*! \class Boob
 *  \brief Brief class description
 *
 *  Detailed description
 */
class Boob
{
public:
    Boob(int x, int y) : x{x}, y{y} {}
    ~Boob() = default;
    void print() { std::cout << x << " " << y << std::endl;}
private:
    int x;
    int y;
};

template <typename T, typename... Args>
class Pool
{
    return Boob(Args&&... args);
}


int main(int argc, const char *argv[])
{
    Boob b{5, 7};
    b.print();
    return 0;
}
