#ifndef COMMAND_COMMAND_H_
#define COMMAND_COMMAND_H_

#include <functional>

#define CMD_BASE(name) \
    void execute(Application*) override;\
    const char* getName() const override {return name;};

namespace sword
{

class Application;

namespace state { class Report; class GraphicsPipelineReport; }

namespace command 
{

class Command;

using SuccessFn = std::function<void(state::Report*)>;

class Command
{
public:
    virtual ~Command() = default;
    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;

    virtual void execute(Application*) = 0;
    virtual const char* getName() const = 0;
    virtual state::Report* makeReport() const { return nullptr; };
    inline bool isAvailable() const {return !inUse;}
    template <typename... Args> void set(Args... args) {}
    void reset() {inUse = false; success_status = false; report = nullptr; successFn = nullptr; }
    void activate() {inUse = true;}
    void setSuccessFn(SuccessFn fn) { successFn = fn; }
    bool succeeded() {return success_status;}
protected:
    Command() = default;
    std::function<void(state::Report*)> successFn{nullptr};
    state::Report* report{nullptr};
    void success() {
        success_status = true;
        if (success_status && successFn)
        {
            report = makeReport();
            std::invoke(successFn, report);
        }}
private:
    bool inUse{false};
    bool success_status{false};
};

}; // namespace command
}; // namespace sword

#endif
