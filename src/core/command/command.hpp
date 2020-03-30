#ifndef COMMAND_COMMAND_H_
#define COMMAND_COMMAND_H_

#define CMD_BASE(name) \
    void execute(Application*) override;\
    const char* getName() const override {return name;};

#include <types/pool.hpp>

namespace sword
{

class Application;

namespace command 
{

class Command
{
public:
    virtual ~Command() = default;
    virtual void execute(Application*) = 0;
    virtual const char* getName() const = 0;
//    virtual const char** getParms() const = 0;
    inline bool isAvailable() const {return !inUse;}
    template <typename... Args> void set(Args... args) {}
    void reset() {inUse = false;}
    void activate() {inUse = true;}
protected:
    Command() = default;
    bool inUse{false};
};

}; // namespace command
}; // namespace sword

#endif
