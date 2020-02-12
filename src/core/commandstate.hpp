#include "../core/event.hpp"
#include "../core/renderer.hpp"
#include <stack>

class ReportStuff;
class Application;
class LoadShaderState;

namespace Command
{
    class Command;
    class AddAttachment;
    class LoadShaders;
};

namespace State
{

class State
{
public:
    virtual Command::Command* handleEvent(Application*, Event* event) const = 0;
    virtual void onEnter(Application*) const = 0;
};

class Director: public State
{
public:
    inline Director(std::vector<Command::Command*> cmds) : cmds{cmds} {}
    Command::Command* handleEvent(Application* app, Event* event) const override;
    void onEnter(Application* app) const override;
private:
    std::vector<Command::Command*> cmds;
};

class LoadShaders: public State
{
public:
    inline LoadShaders(Command::LoadShaders* cmd) : cmd{cmd} {}
    Command::Command* handleEvent(Application* app, Event* event) const override;
    void onEnter(Application* app) const override;
private:
    Command::LoadShaders* const cmd;
};

class AddAttachment: public State
{
public:
    inline AddAttachment(Command::AddAttachment* cmd) : cmd{cmd} {}
    Command::Command* handleEvent(Application* app, Event* event) const override;
    void onEnter(Application* app) const override;
private:
    Command::AddAttachment* const cmd;
};

class Paint : public State
{
public:
    inline Paint(std::vector<Command::Command*> cmds) : cmds{cmds} {}
    Command::Command* handleEvent(Application* app, Event* event) const override;
    void onEnter(Application* app) const override;
private:
    std::vector<Command::Command*> cmds;
};

};

namespace Command
{

class Command
{
public:
    friend Application;
    virtual State::State* execute(Application*) = 0;
    virtual const char* getName() const = 0;
};

class LoadShaders: public Command
{
public:
    friend class State::LoadShaders;
    inline LoadShaders() : nState{this} {}
    inline const char* getName() const override {return "loadShaders";}
    State::State* execute(Application*) override;
    inline uint8_t getStage() const {return stage;}
private:
    uint8_t stage = 0;
    State::LoadShaders nState;
    //args
    bool isFrag{true};
    std::vector<std::string> shaderNames;
};

class AddAttachment: public Command
{
public:
    friend class State::AddAttachment;
    inline AddAttachment() : nState{this} {}
    inline const char* getName() const override {return "addAttachment";}
    State::State* execute(Application*) override;
    void loadArgs(std::string attachmentName, bool isSampled, bool isTransferSrc);
private:
    bool pending{false};
    State::AddAttachment nState;
    //args
    std::string attachmentName;
    bool isSampled;   
    bool isTransferSrc;
};

class OpenWindow : public Command
{
public:
    inline const char* getName() const override {return "openWindow";}
    State::State* execute(Application*) override;
};

class Stamp : public Command
{
public:
    inline const char* getName() const override {return "stamp";}
    inline State::State* execute(Application*) override {return nullptr;}
    void loadArgs(const uint16_t x,const uint16_t y);
private:
    bool active{false};
    uint16_t x, y;
};

class EnterPaint : public Command
{
public:
    inline EnterPaint(State::Paint* state) : state{state} {}
    inline const char* getName() const override {return "enterPaint";}
    State::State* execute(Application*) override;
private:
    State::Paint* state;
};

};

//TODO: implement an iterator for this so we don't need
//to have the friend class
class StateStack
{
public:
    friend class Application;
    inline void push(State::State* state) {states.push_back(state);}
    inline void pop() {states.pop_back();}
    inline State::State* top() const {return states.back();}
private:
    std::vector<State::State*> states;
};

class Application
{
public:
    Application(uint16_t w, uint16_t h);
    void setVocabulary(std::vector<std::string>);
    void run();
    void popState();

    Context context;
    XWindow window;
    Renderer renderer;
    EventHandler ev;
    vk::Extent2D offscreenDim;

private:
    StateStack stateStack;
    std::stack<Command::Command*> cmdStack;

    State::Director dirState;
    State::Paint paintState;

    Command::AddAttachment aaCmd;
    Command::LoadShaders lsCmd;
    Command::OpenWindow ow;
    Command::Stamp stampCmd;
    Command::EnterPaint epCmd;
};
