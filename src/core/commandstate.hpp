#include "../core/event.hpp"
#include "../core/renderer.hpp"
#include <stack>
#include <map>

class Application;
class AddAttachment;

#define STATE_BASE(name) \
    void handleEvent(Event* event, EditStack*, CommandStack*) override;\
    virtual const char* getName() const override {return name;}

#define CMD_BASE(name) \
    void execute(Application*) override;\
    static const char* getName() {return name;}

//TODO:
//want to make it so that we can call commands up the state stack
//we will impose some limitations on the stack in accordance with this change
//1) Each state can have ONLY ONE CHILD on the stack at any given time. 
//   The thought here is to prevent 
template <typename T>
class OptionMap
{
public:
    using Element = std::pair<std::string, T>;
    OptionMap(
            std::initializer_list<Element> avail,
            std::initializer_list<Element> reserved) : 
        optionsAvailable(avail),
        optionsReserved(reserved) {}
    inline T findOption(const std::string& s) const 
    {
        T t;
        for (const auto& item : optionsAvailable) 
            if (item.first == s)
                t = item.second;
        return t; //should be 0 if not found 
    }
    inline void swapOut(T t) 
    {
        const size_t size = optionsAvailable.size();
        for (int i = 0; i < size; i++) 
           if (optionsAvailable[i].second == t)
           {
               Element element = optionsAvailable[i];
               optionsReserved.push_back(element);
               optionsAvailable.erase(optionsAvailable.begin() + i); 
           }
    }
    inline void remove(T t)
    {
        size_t size = optionsAvailable.size();
        for (int i = 0; i < size; i++) 
           if (optionsAvailable[i].second == t)
               optionsAvailable.erase(optionsAvailable.begin() + i); 
        size = optionsReserved.size();
        for (int i = 0; i < size; i++) 
           if (optionsReserved[i].second == t)
               optionsReserved.erase(optionsReserved.begin() + i); 
    }
    inline std::vector<std::string> getStrings() const
    {
        std::vector<std::string> vec;
        vec.reserve(optionsAvailable.size());
        for (const auto& item : optionsAvailable) 
            vec.push_back(item.first);
        return vec;
    }
    inline void swapIn(T t)
    {
        const size_t size = optionsReserved.size();
        for (int i = 0; i < size; i++) 
        {
            if (optionsReserved[i].second == t)
            {
                Element element = optionsReserved[i];
                optionsAvailable.push_back(element);
                optionsReserved.erase(optionsReserved.begin() + i);
            }   
        }
    }
private:
    std::vector<std::pair<std::string, T>> optionsAvailable;
    std::vector<std::pair<std::string, T>> optionsReserved;
};

namespace rpt
{

enum class ReportType : uint8_t
{
    Shader,
    Renderpass,
    Pipeline,
    RenderpassInstance,
    RenderCommand
};

class Report
{
public:
    virtual void operator()() const = 0;
    virtual const std::string getObjectName() const = 0;
    virtual ReportType getType() const = 0;
};

class ShaderReport : public Report
{
public:
    inline ShaderReport(std::string n, const char* t, int i0, int i1, float f0, float f1) :
        name{n}, type{t}, specint0{i0}, specint1{i1}, specfloat0{f0}, specfloat1{f1} {}
    inline void operator()() const  override
    {
        std::cout << "================== Shader Report ==================" << std::endl;
        std::cout << "Name:                  " << name << std::endl;
        std::cout << "Type:                  " << type << std::endl;
        std::cout << "Spec Constant Int 0:   " << specint0 << std::endl;
        std::cout << "Spec Constant Int 1:   " << specint1 << std::endl;
        std::cout << "Spec Constant Float 0: " << specfloat0 << std::endl;
        std::cout << "Spec Constant Float 1: " << specfloat1 << std::endl;
    }
    inline const std::string getObjectName() const override {return name;};
    inline ReportType getType() const override {return ReportType::Shader;}
    inline void setSpecFloat(int index, float val)
    {
        if (index == 0) specfloat0 = val;
        if (index == 1) specfloat1 = val;
    }

private:
    const std::string name{"unitilialized"};
    const char* type{"uninilialized"};
    int specint0{0};
    int specint1{0};
    float specfloat0{0};
    float specfloat1{0};
};

class GraphicsPipelineReport : public Report
{
public:
    inline GraphicsPipelineReport(
            std::string parm1,
            std::string parm2,
            std::string parm3,
            std::string parm4,
            std::string parm5,
            int i1,
            int i2,
            uint32_t ui1,
            uint32_t ui2,
            bool is3d) :
        name{parm1}, pipelineLayout{parm2}, vertshader{parm3}, fragshader{parm4},
        renderpass{parm5}, regionX{i1}, regionY{i2}, areaX{ui1}, areaY{ui2}, is3d{is3d} {}
    inline void operator()() const override
    {
        std::cout << "================== Graphics Pipeline Report ==================" << std::endl;
        std::cout << "Name:            " << name << std::endl;
        std::cout << "Pipeline layout: " << pipelineLayout << std::endl;
        std::cout << "Vert Shader:     " << vertshader << std::endl;
        std::cout << "Frag Shader:     " << fragshader << std::endl;
        std::cout << "Renderpass:      " << renderpass << std::endl;
        std::cout << "Coords:         (" << regionX << ", " << regionY << ")" << std::endl;
        std::cout << "Area:           (" << areaX << ", " << areaY << ")" << std::endl;
        std::cout << "Is it 3d?        " << is3d << std::endl;
    }
    inline const std::string getObjectName() const override {return name;}
    inline ReportType getType() const override {return ReportType::Pipeline;}
private:
    std::string name{"default"};
    std::string pipelineLayout{"default"};
    std::string vertshader{"default"};
    std::string fragshader{"default"};
    std::string renderpass{"default"};
    int regionX;
    int regionY;
    uint32_t areaX;
    uint32_t areaY;
    bool is3d{false};
};

class RenderpassReport : public Report
{
public:
    inline RenderpassReport(std::string n) :
        name{n} {}
    inline void operator()() const override
    {
        std::cout << "================== Renderpass Report ==================" << std::endl;
        std::cout << "Name:                  " << name << std::endl;
    }
    inline const std::string getObjectName() const override {return name;}
    inline ReportType getType() const override {return ReportType::Renderpass;}
private:
    const std::string name{"unitilialized"};
};

class RenderpassInstanceReport : public Report
{
public:
    inline RenderpassInstanceReport(
            std::string attachName,
            std::string rpassName,
            std::string pipeName,
            int id) :
        attachmentName{attachName},
        renderpassName{rpassName},
        pipelineName{pipeName},
        id{id} {}
    inline void operator()() const override
    {
        std::cout << "============== Renderpass Instance Report =============" << std::endl;
        std::cout << "Index:           " << id << std::endl;
        std::cout << "Attachment Name: " << attachmentName << std::endl;
        std::cout << "Renderpass Name: " << renderpassName << std::endl;
        std::cout << "Pipeline Name:   " << pipelineName << std::endl;
    }
    inline ReportType getType() const override {return ReportType::RenderpassInstance;}
    inline const std::string getObjectName() const override {return std::to_string(id);}
private:
    std::string attachmentName;
    std::string renderpassName;
    std::string pipelineName;
    int id;
};

class RenderCommandReport : public Report
{
public:
    inline RenderCommandReport(int cmdIndex, std::vector<uint32_t> rpiIndices) :
        cmdIndex{cmdIndex}, rpiIndices{rpiIndices} {}
    inline void operator()() const override
    {
        std::cout << "============= Render Commmand Report =============" << std::endl;
        std::cout << "Command Index:        " << cmdIndex << std::endl;
        std::cout << "Renderpass Instances: ";
        for (const auto& i : rpiIndices) 
        {
            std::cout << i << ", ";
        }
        std::cout << std::endl;
    }
    inline ReportType getType() const override {return ReportType::RenderCommand;}
    inline const std::string getObjectName() const override {return std::to_string(cmdIndex);}
private:
    int cmdIndex;
    std::vector<uint32_t> rpiIndices;
};

};

template <class T>
class Stack
{
public:
    friend class Application;
    inline void push(T&& item) {items.push_back(std::forward<T>(item));}
    inline void pop() {items.pop_back();}
    inline T top() const {return items.back();}
    inline bool empty() const {return items.empty();}
    inline void print() const {for (const auto& item : items) std::cout << item->getName() << std::endl;}
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

namespace State
{
class State;
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

class RefreshState : public Command
{
public:
    CMD_BASE("refreshState");
    inline void set(State::State* state) {Command::set(); this->state = state;}
private:
    State::State* state{nullptr};
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

class SetSpecFloat: public Command
{
public:
    CMD_BASE("setSpecFloat");
    inline void set(std::string name, std::string t, float first, float second) {
        Command::set(); shaderName = name; type = t; x = first; y = second;}
private:
    std::string shaderName;
    std::string type;
    float x{0};
    float y{0};
};

class SetSpecInt: public Command
{
public:
    CMD_BASE("setSpecInt");
    inline void set(std::string name, std::string t, int first, int second) {
        Command::set(); shaderName = name; type = t; x = first; y = second;}
private:
    std::string shaderName;
    std::string type;
    int x{0};
    int y{0};
};

class AddAttachment: public Command
{
public:
    CMD_BASE("addAttachment");
    inline void set(std::string name, int x, int y, bool sample, bool transfer)
    {
        assert(x > 0 && y > 0 && "Bad attachment size");
        Command::set();
        attachmentName = name;
        isSampled = sample;
        isTransferSrc = transfer;
        dimensions.setWidth(x);
        dimensions.setHeight(y);
    }
private:
    std::string attachmentName;
    vk::Extent2D dimensions{{500, 500}};
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

class CreateDescriptorSetLayout : public Command
{
public:
    CMD_BASE("createDescriptorSetLayout");
private:
    std::string name{"default"};
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

class CreatePipelineLayout : public Command
{
public:
    CreatePipelineLayout();
    CMD_BASE("createPipelineLayout")
private:
    std::string layoutname{"foo"};
};

class PrepareRenderFrames : public Command
{
public:
    CMD_BASE("prepareRenderFrames");
    inline void set(XWindow* window) {Command::set(); this->window = window;}
private:
    XWindow* window{nullptr};
};

class CreateGraphicsPipeline : public Command
{
public:
    CMD_BASE("createGraphicsPipeline");
    inline void set(
            std::string parm1,
            std::string parm2,
            std::string parm3,
            std::string parm4,
            std::string parm5,
            vk::Rect2D renderAreaSet,
            bool has3dGeo) 
    {
        name = parm1;
        pipelineLayout = parm2;
        vertshader = parm3;
        fragshader = parm4;
        renderpass = parm5;
        renderArea = renderAreaSet;
        is3d = has3dGeo;
    }
private:
    std::string name{"default"};
    std::string pipelineLayout{"default"};
    std::string vertshader{"default"};
    std::string fragshader{"default"};
    std::string renderpass{"default"};
    vk::Rect2D renderArea{{0, 0}, {100, 100}};
    bool is3d{false};
};

class CreateSwapchainRenderpass : public Command
{
public:
    CMD_BASE("create_swapchain_render_pass");
    inline void set(std::string name) {Command::set(); rpassName = name;}
private:
    std::string rpassName;
};

class CreateRenderpassInstance : public Command
{
public:
    CMD_BASE("createRenderpassInstance");
    inline void set(
            std::string attachName,
            std::string renderpassName,
            std::string pipelineName) 
    {
        attachment = attachName;
        renderpass = renderpassName;
        pipeline = pipelineName;
    }
private:
    std::string attachment;
    std::string renderpass;
    std::string pipeline;
};

class RecordRenderCommand : public Command
{
public:
    CMD_BASE("recordRenderCommand");
    inline void set(int index, std::vector<uint32_t> renderpassInstances)
    {
        this->cmdBufferId = index;
        this->renderpassInstances = renderpassInstances;
    }
private:
    int cmdBufferId;
    std::vector<uint32_t> renderpassInstances;
};

class Render : public Command
{
public:
    CMD_BASE("render");
    inline void set(int renderCommandId, bool updateUBO)
    {
        this->renderCommandId = renderCommandId;
        this->updateUBO = updateUBO;
    }
private:
    int renderCommandId{0};
    bool updateUBO{false};
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

class EditStack;

//TODO: May want to give Director the ability to act even if an
//      event has been handled. For instance, if we have a 
//      director -> renderermanager -> painter type stack
//      we mayy want to allow the renderermanager to push
//      a state. For instance, it may be useful to modify 
//      the attachments while the painter is active.
//      we can do this, but we don't want a 
//      director -> renderermanager -> painter -> attachmentmanager
//      stack. So even though the event was handled, we may
//      allow the director to do a swap on the stack ie swap 
//      attachmentmanager with painter. so we will have to put the 
//      if (!eventHandled) check inside the handleEvent() functions
//      also, we might allow states to handle abortions if they're state
//      is not null (not sure about this though, the idea is to get out of
//      pending states)
namespace State
{

class State;
typedef ReverseStack<State*> StateStack;

typedef std::vector<std::string> Vocab;

using namespace rpt;

class State
{
public:
    virtual void handleEvent(Event* event, EditStack*, CommandStack*) = 0;
    void onEnter(Application*);
    void onExit(Application*);
    void onResume(Application* app);
    void onPush(Application* app);
    virtual void refresh(Application* app);
    virtual const char* getName() const = 0;
protected:
    void pushState(State* state, EditStack* edits);
    void popState(EditStack* edits);
    virtual void removeDynamicOps() {}
    virtual void addDynamicOps() { std::cout << "Called Base dynOps" << std::endl;}
    virtual void onResumeExt(Application*) {}
    virtual void onPushExt(Application*) {}
    virtual void onEnterExt(Application*) {}
    virtual void onExitExt(Application*) {}

    inline static Command::Pool<Command::RefreshState> refreshPool{1};
    State* activeChild{nullptr};
    Vocab vocab;
};

class ShaderManager: public State
{
public:
    STATE_BASE("shadermanager");
    ShaderManager() {vocab = opMap.getStrings();}
    std::vector<const Report*> getReports() const
    {
        std::vector<const Report*> reports;
        for (const auto& item : shaderReports) 
            reports.push_back(item.second.get());
        return reports;
    }
private:
    void setShaderVocab();
    Command::Pool<Command::LoadFragShader> loadFragPool{10};
    Command::Pool<Command::LoadVertShader> loadVertPool{10};
    Command::Pool<Command::SetSpecFloat> ssfPool{10};
    Command::Pool<Command::SetSpecInt> ssiPool{10};
    enum class Option {loadFragShaders = 1, loadVertShaders, shaderReport, setSpecInts, setSpecFloats};
    enum class Mode {null, loadFragShaders, loadVertShaders, setSpecInts, setSpecFloats};
    OptionMap<Option> opMap{
        {
            {"loadfragshaders", Option::loadFragShaders},
            {"loadvertshaders", Option::loadVertShaders},
            {"report", Option::shaderReport},
            {"setspecints", Option::setSpecInts},
            {"setspecfloats", Option::setSpecFloats},
            {"setwindowresolution", Option::setSpecFloats}
        },{
        }};

    Mode mode{Mode::null};
    std::map<std::string, std::unique_ptr<ShaderReport>> shaderReports;
};

class AddAttachment: public State
{
public:
    STATE_BASE("addAttachment");
    AddAttachment() {vocab = opMap.getStrings();};
private:
    Command::Pool<Command::AddAttachment> addAttPool{20};
    int width, height;
    enum class Option : uint8_t {setdimensions = 1, fulldescription, addsamplesources};
    enum class Mode : uint8_t {null, setdimensions, fulldescription, addsamplesources};
    OptionMap<Option> opMap {
        {
            {"setDimensions", Option::setdimensions},
            {"fulldescription", Option::fulldescription},
            {"addSampleSources", Option::addsamplesources}
        },{
        }};

    Mode mode{Mode::null};
};

class Paint : public State
{
public:
    STATE_BASE("paint")
private:
};

class RenderpassManager: public State
{
public:
    STATE_BASE("renderpassManager");
    RenderpassManager() {vocab = opMap.getStrings();}
    std::vector<const Report*> getReports() const 
    {
        std::vector<const Report*> reports;
        for (const auto& item : renderpassReports) 
            reports.push_back(item.get());
        return reports;
    }

private:    
    Command::Pool<Command::CreateSwapchainRenderpass> csrPool{1};
    enum class Option : uint8_t {createSwapRenderpass = 1, report};
    enum class Mode : uint8_t {null, createSwapRenderpass};
    OptionMap<Option> opMap{
        {
        {"create_swap_renderpass", Option::createSwapRenderpass},
        {"report", Option::report},
        },{
        }};
    std::vector<std::unique_ptr<RenderpassReport>> renderpassReports;
    Mode mode{Mode::null};
};

class PipelineManager : public State
{
public:
    STATE_BASE("pipelineManager");
    PipelineManager() {vocab = opMap.getStrings();}
    inline void setCGPVocab(Vocab v) { cgpVocab = v;}
    std::vector<const Report*> getReports() const
    {
        std::vector<const Report*> reports;
        for (const auto& item : gpReports) 
            reports.push_back(item.second.get());
        return reports;
    }
private:
    Command::Pool<Command::CreateGraphicsPipeline> cgpPool{10};
    enum class Option : uint8_t {createGraphicsPipeline = 1, report};
    enum class Mode : uint8_t {null, createGraphicsPipeline};
    OptionMap<Option> opMap{
        {
        {"createGraphicsPipeline", Option::createGraphicsPipeline},
        {"report", Option::report }
        },{
        }};

    Mode mode{Mode::null};
    std::map<std::string, std::unique_ptr<GraphicsPipelineReport>> gpReports;

    Vocab cgpVocab{};
};

class CreateDescriptorSetLayout : public State
{
public:
    STATE_BASE("createDescriptorSetLayout")
private:
    Command::Pool<Command::CreateDescriptorSetLayout> cdslPool{1};
    std::vector<vk::DescriptorSetLayoutBinding> pendingBindings;
};

class DescriptorManager : public State
{
public:
    STATE_BASE("descriptorManager");
private:
    CreateDescriptorSetLayout cdsl;
    enum class Option {createDescriptorSetLayout = 1};
};

class RendererManager : public State
{
public:
    inline RendererManager(XWindow& the_window) : window{the_window} {vocab = opMap.getStrings();}
    STATE_BASE("rendererManager");
    void onResumeExt(Application* app) override;
    void onPushExt(Application* app) override;
private:
    Command::Pool<Command::PrepareRenderFrames> prfPool{1};
    Command::Pool<Command::CreatePipelineLayout> cplPool{1};
    Command::Pool<Command::OpenWindow> owPool{1};
    Command::Pool<Command::CreateRenderpassInstance> criPool{5};
    Command::Pool<Command::RecordRenderCommand> rrcPool{5};
    Command::Pool<Command::Render> renderPool{5};
    RenderpassManager rpassManager;
    PipelineManager pipelineManager;
    ShaderManager shaderManager;
    AddAttachment addAttachState;

    enum class Option : uint8_t {render = 1, recordRenderCmd, rpiReport, printReports, createRPI, shaderManager, openWindow, addattachState, prepRenderFrames, createPipelineLayout, rpassManager, pipelineManager, all};
    enum class Mode : uint8_t {null, createRPI, render, recordRenderCmd};
    OptionMap<Option> opMap{
        {
            {"createPipelineLayout", Option::createPipelineLayout},
            {"openWindow", Option::openWindow},
            {"addAttachState", Option::addattachState},
            {"shaderManager", Option::shaderManager},
            {"all", Option::all},
            {"printReports", Option::printReports},
            {"rpiReports", Option::rpiReport}
        },{
            {"rpassManager", Option::rpassManager},
            {"prepRenderFrames", Option::prepRenderFrames},
            {"pipelineManager", Option::pipelineManager},
            {"createRenderpassInstance", Option::createRPI},
            {"render", Option::render},
            {"recordRenderCmd", Option::recordRenderCmd}
        }};

    static constexpr std::array<Option, 8> dynamicOptions{Option::shaderManager, Option::addattachState, Option::rpassManager, Option::pipelineManager, Option::all, Option::createRPI, Option::recordRenderCmd, Option::render};
    inline void removeDynamicOps() override { for (const auto& op : dynamicOptions) opMap.swapOut(op); vocab = opMap.getStrings();}
    inline void addDynamicOps() override { for (const auto& op : dynamicOptions) opMap.swapIn(op); vocab = opMap.getStrings();}

    Mode mode{Mode::null};
    XWindow& window;

    std::vector<const Report *> reports;
    std::vector<std::unique_ptr<RenderpassInstanceReport>> rpiReports;
    std::vector<std::unique_ptr<RenderCommandReport>> rcReports;
};

class Director: public State
{
public:
    STATE_BASE("director");
    Director(XWindow& window, const StateStack& ss) : initRenState{window}, stateStack{ss} {vocab = opMap.getStrings();}
    void onPushExt(Application*) override;
    void onResumeExt(Application*) override;
private:
    RendererManager initRenState;
    Paint paint;
    enum class Option : uint8_t {initRenState = 1, paint, printStack};
    OptionMap<Option> opMap{
        {
            {"rendererManager", Option::initRenState},
            {"printStack", Option::printStack},
            {"painter", Option::paint}
        },{
        }};

    static constexpr std::array<Option, 2> dynamicOptions{Option::paint, Option::initRenState};
    inline void removeDynamicOps() override { for (const auto& op : dynamicOptions) opMap.swapOut(op);}
    inline void addDynamicOps() override { for (const auto& op : dynamicOptions) opMap.swapIn(op);}

    const StateStack& stateStack;
};

};

typedef ReverseStack<State::State*> StateStack;

enum class EditType : uint8_t
{
    push,
    remove,
};

class EditStack : protected ReverseStack<State::State*>
{
friend class State::Director;
public:
    inline void pushState(State::State* ptr) {push(std::forward<State::State*>(ptr));}
    inline auto begin() {return ReverseStack<State::State*>::begin();}
    inline auto end() {return ReverseStack<State::State*>::end();}
    inline auto clear() { items.clear();}
private:
    inline void popState() {push(nullptr);}
};

class Application
{
public:
    Application(uint16_t w, uint16_t h, const std::string logfile);
    void setVocabulary(State::Vocab);
    void run();
    void popState();
    void pushState(State::State* const);
    void createPipelineLayout();
    void loadDefaultShaders();
    void createDefaultRenderPasses();
    void initUBO();

    void readEvents(std::ifstream&);
    void recordEvent(Event* event, std::ofstream& os);

    Context context;
    XWindow window;
    Renderer renderer;
    EventHandler ev;
    vk::Extent2D offscreenDim;
    vk::Extent2D swapDim;;

private:
    std::string eventlog;
    bool recordevents{true};
    bool readevents{false};
    
    StateStack stateStack;
    EditStack stateEdits;
    CommandStack cmdStack;
    FragmentInput fragInput;

    State::Director dirState;
};
