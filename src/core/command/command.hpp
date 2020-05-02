#ifndef COMMAND_COMMAND_H_
#define COMMAND_COMMAND_H_

#define CMD_BASE(name) \
    void execute(Application*) override;\
    const char* getName() const override {return name;};

namespace sword
{

class Application;

namespace state { class Report; class GraphicsPipelineReport; }

namespace command 
{

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
    void reset() {inUse = false; success_status = false;}
    void activate() {inUse = true;}
    bool succeeded() {return success_status;}
protected:
    Command() = default;
    void success() {success_status = true;}

private:
    bool inUse{false};
    bool success_status{false};
};

}; // namespace command
}; // namespace sword

#endif
