#include "../core/event.hpp"
#include "../core/renderer.hpp"
#include <stack>

class Application;
class AddAttachment;
class StackHolder;

#define STATE_BASE(name) \
    Command::Command* handleEvent(Event* event, Stack<State*>*) override;\
    void onEnter(Application* app) override;\
    inline const char* getName() const override {return name;}

#define CMD_BASE(name) \
    void execute(Application*) override;\
    inline const char* getName() const override {return name;}

//TODO: implement an iterator for this so we don't need
//to have the friend class
template <class T>
class Stack
{
public:
    friend class Application;
    inline void push(T const item) {items.push_back(item);}
    inline void pop() {items.pop_back();}
    inline T top() const {return items.back();}
    inline bool empty() const {return items.empty();}
private:
    std::vector<T> items;
};


struct FragmentInput
{
    float time;
	float mouseX{0};
	float mouseY{0};	
    float r{1.};
    float g{1.};
    float b{1.};
    float a{1.};
    float brushSize{1.};
    glm::mat4 xform{1.};
};

namespace Command
{

class Command
{
public:
    virtual void execute(Application*) = 0;
    virtual const char* getName() const = 0;
};

class LoadFragShaders: public Command
{
public:
    inline const char* getName() const override {return "loadFragShaders";}
    void execute(Application*) override;
    bool isFrag{true};
    std::vector<std::string> shaderNames;
private:
};

class AddAttachment: public Command
{
public:
    inline const char* getName() const override {return "addAttachment";}
    void execute(Application*) override;
    void loadArgs(std::string attachmentName, bool isSampled, bool isTransferSrc);
    std::string attachmentName;
    bool isSampled{false};   
    bool isTransferSrc{false};
private:
};

class OpenWindow : public Command
{
public:
    CMD_BASE("openWindow")
};

class Stamp : public Command
{
public:
    inline const char* getName() const override {return "stamp";}
    inline void execute(Application*) override {}
    void loadArgs(const uint16_t x,const uint16_t y);
private:
    uint16_t x{0}, y{0};
};

class EnterPaint : public Command
{
public:
    inline const char* getName() const override {return "enterPaint";}
    void execute(Application*) override {};
};

class SetOffscreenDim : public Command
{
public:
    CMD_BASE("setOffscreenDim")
    uint16_t w{0}, h{0};
};

class CreatePipelineLayout : public Command
{
public:
    CreatePipelineLayout();
    CMD_BASE("createPipelineLayout")
    std::string layoutname{"foo"};
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
private:
};

class PrepareRenderFrames : public Command
{
public:
    CMD_BASE("prepareRenderFrames")
    XWindow* window{nullptr};
private:
};

};

namespace State
{

class State
{
public:
    virtual Command::Command* handleEvent(Event* event, Stack<State*>*) = 0;
    virtual void onEnter(Application*) = 0;
    virtual const char* getName() const = 0;
};

class Director: public State
{
public:
    inline Director(std::vector<State*> states, std::vector<Command::Command*> cmds) : states{states}, cmds{cmds} {}
    STATE_BASE("director")
private:
    std::vector<Command::Command*> cmds;
    std::vector<State*> states;
};

class LoadFragShaders: public State
{
public:
    inline LoadFragShaders(Command::LoadFragShaders* cmd) : cmd{cmd} {}
    STATE_BASE("loadFragShaders")
private:
    Command::LoadFragShaders* const cmd{nullptr};
    uint8_t stage{0};
};

class AddAttachment: public State
{
public:
    inline AddAttachment(Command::AddAttachment* cmd) : cmd{cmd} {}
    STATE_BASE("addAttachment")
private:
    Command::AddAttachment* const cmd{nullptr};
};

class Paint : public State
{
public:
    inline Paint(std::vector<Command::Command*> cmds) : cmds{cmds} {}
    STATE_BASE("paint")
private:
    std::vector<Command::Command*> cmds;
};

class SetOffscreenDim : public State
{
public:
    inline SetOffscreenDim(Command::SetOffscreenDim* cmd) : cmd{cmd} {}
    STATE_BASE("setOffscreenDim")
private:
    Command::SetOffscreenDim* cmd{nullptr};
};

class InitRenderer : public State
{
public:
    inline InitRenderer(XWindow& the_window) : window{the_window} {}
    STATE_BASE("initRenderer")
private:
    Command::PrepareRenderFrames prepRFrames;
    Command::CreatePipelineLayout createPipelineLayout;
    Command::LoadFragShaders loadShaders;
    std::vector<Command::Command*> cmds{&createPipelineLayout, &prepRFrames, &loadShaders};
    XWindow& window;
    bool preparedFrames{false};
};

};

class Application
{
public:
    Application(uint16_t w, uint16_t h);
    void setVocabulary(std::vector<std::string>);
    void run();
    void popState();
    void pushState(State::State* const);
    void pushCmd(Command::Command* const);
    void prepareRenderFrames();
    void createPipelineLayout();
    void loadDefaultShaders();
    void createDefaultRenderPasses();
    void initUBO();

    void readEvents(std::ifstream&);

    Context context;
    XWindow window;
    Renderer renderer;
    EventHandler ev;
    vk::Extent2D offscreenDim;

private:
    Stack<State::State*> stateStack;
    Stack<Command::Command*> cmdStack;
    Stack<State::State*> stateEdits;
    FragmentInput fragInput;

    State::Director dirState;
    State::Paint paintState;
    State::AddAttachment aaState;
    State::LoadFragShaders lsState;
    State::SetOffscreenDim sodimState;
    State::InitRenderer initRen{window};

    Command::AddAttachment aaCmd;
    Command::LoadFragShaders lsCmd;
    Command::OpenWindow ow;
    Command::Stamp stampCmd;
    Command::EnterPaint epCmd;
    Command::SetOffscreenDim sodimCmd;
};
