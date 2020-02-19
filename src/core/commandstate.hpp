#include "../core/event.hpp"
#include "../core/renderer.hpp"
#include <stack>
#include <map>

class Application;
class AddAttachment;

#define STATE_BASE(name) \
    void handleEvent(Event* event, StateStack*, CommandStack*) override;\
    void onEnter(Application* app) override;\
    static const char* getName() {return name;}

#define CMD_BASE(name) \
    void execute(Application*) override;\
    static const char* getName() {return name;}

template <class T>
class Stack
{
public:
    friend class Application;
    inline void push(T&& item) {items.push_back(std::forward<T>(item));}
    inline void pop() {items.pop_back();}
    inline T top() const {return items.back();}
    inline bool empty() const {return items.empty();}
protected:
    std::vector<T> items;
};

template <class T>
class ForwardStack : public Stack<T>
{
public:
    inline auto begin() {return this->items.begin();}
    inline auto end() {return this->items.end();}
};

template <class T>
class ReverseStack : public Stack<T>
{
public:
    inline auto begin() {return this->items.rbegin();}
    inline auto end() {return this->items.rend();}
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
    virtual ~Command() = default;
    virtual void execute(Application*) = 0;
    static const char* getName() {return "commandBase";}
    inline bool isAvailable() const {return !inUse;}
    template <typename... Args> inline void set(Args... args) {inUse = true;}
    void reset() {inUse = false;}
protected:
    Command() = default;
    bool inUse{false};
};

class LoadFragShader: public Command
{
public:
    CMD_BASE("loadFragShader");
    inline void set(std::string name) {Command::set(); shaderName = name;}
private:
    std::string shaderName;
};


class LoadVertShader: public Command
{
public:
    CMD_BASE("loadVertShader");
    inline void set(std::string name) {Command::set(); shaderName = name;}
private:
    std::string shaderName;
};

class AddAttachment: public Command
{
public:
    CMD_BASE("addAttachment");
    inline void set(std::string name, bool sample, bool transfer)
    {
        Command::set();
        attachmentName = name;
        isSampled = sample;
        isTransferSrc = transfer;
    }
private:
    std::string attachmentName;
    bool isSampled{false};   
    bool isTransferSrc{false};
};

class OpenWindow : public Command
{
public:
    CMD_BASE("openWindow")
};

class SetOffscreenDim : public Command
{
public:
    CMD_BASE("setOffscreenDim");
private:
    uint16_t w{0}, h{0};
};

class CreatePipelineLayout : public Command
{
public:
    CreatePipelineLayout();
    CMD_BASE("createPipelineLayout")
private:
    std::string layoutname{"foo"};
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

class PrepareRenderFrames : public Command
{
public:
    CMD_BASE("prepareRenderFrames");
    inline void set(XWindow* window) {Command::set(); this->window = window;}
private:
    XWindow* window{nullptr};
};

template <typename T>
class Pool
{
public:
    template <typename P> using Pointer = std::unique_ptr<P, std::function<void(P*)>>;

    Pool(size_t size) : size{size}, pool(size) {}

    template <typename... Args> Pointer<Command> request(Args... args)
    {
        for (int i = 0; i < size; i++) 
            if (pool[i].isAvailable())
            {
                pool[i].set(args...);
                Pointer<Command> ptr{&pool[i], [](Command* t)
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

    static const char* getName() {return T::getName();}

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

typedef ForwardStack<std::unique_ptr<Command::Command, std::function<void(Command::Command*)>>> CommandStack;

namespace State
{

class State;
typedef ReverseStack<State*> StateStack;

typedef std::vector<std::string> Vocab;

class State
{
public:
    virtual void handleEvent(Event* event, StateStack*, CommandStack*) = 0;
    virtual void onEnter(Application*) = 0;
};

class LoadShaders: public State
{
public:
    STATE_BASE("loadFragShaders");
private:
    Command::Pool<Command::LoadFragShader> loadFragPool{25};
    Command::Pool<Command::LoadVertShader> loadVertPool{25};
    uint8_t stage{0};
};

class AddAttachment: public State
{
public:
    STATE_BASE("addAttachment");
private:
    Command::Pool<Command::AddAttachment> addAttPool{1};
};

class Paint : public State
{
public:
    STATE_BASE("paint")
private:
};

class InitRenderer : public State
{
public:
    inline InitRenderer(XWindow& the_window) : window{the_window} {}
    STATE_BASE("initRenderer");
private:
    Command::Pool<Command::PrepareRenderFrames> prfPool{1};
    Command::Pool<Command::CreatePipelineLayout> cplPool{1};
    const std::vector<std::string> vocab{
        prfPool.getName(), cplPool.getName()};
    XWindow& window;
    bool preparedFrames{false};
    bool createdLayout{false};
};

class Director: public State
{
public:
    STATE_BASE("director");
    Director(XWindow& window) : initRenState{window} {}
    template <typename T> inline void pushStateFn(std::string& str, T& t, StateStack* stack) const
    {
        if (str == t.getName())
            stack->push(&t);
    }
private:
    Command::Pool<Command::OpenWindow> owPool{1};
    AddAttachment addAttachState;
    InitRenderer initRenState;
    LoadShaders loadShaders;
    const std::vector<std::string> vocab{
        initRenState.getName(), addAttachState.getName(), owPool.getName(), loadShaders.getName()};
};

};

typedef ReverseStack<State::State*> StateStack;

class Application
{
public:
    Application(uint16_t w, uint16_t h, bool recordevents, bool readevents);
    void setVocabulary(State::Vocab);
    void run();
    void popState();
    void pushState(State::State* const);
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
    vk::Extent2D swapDim;;

private:
    const bool recordevents;
    const bool readevents;
    
    StateStack stateStack;
    StateStack stateEdits;
    CommandStack cmdStack;
    FragmentInput fragInput;

    State::Director dirState;
};
