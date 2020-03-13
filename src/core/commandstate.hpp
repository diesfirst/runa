#include "../core/event.hpp"
#include "../core/renderer.hpp"
#include <stack>
#include <map>
#include <sstream>

class Application;
class AddAttachment;

#define STATE_BASE(name) \
    void handleEvent(Event* event) override;\
    virtual const char* getName() const override {return name;}

#define CMD_BASE(name) \
    void execute(Application*) override;\
    const char* getName() override {return name;};


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

//TODO:
//want to make it so that we can call commands up the state stack
//we will impose some limitations on the stack in accordance with this change
//1) Each state can have ONLY ONE CHILD on the stack at any given time. 
//   The thought here is to prevent 
template <typename S, typename T>
class SmallMap
{
public:
    using Element = std::pair<S, T>;
    SmallMap(std::initializer_list<Element> avail) : options{avail} {}
    inline std::optional<T> findOption(CommandLineEvent* event) const
    {
        std::string input = event->getInput();
        std::stringstream instream{input};
        instream >> input;
        return findOption(input);
    }
    inline std::optional<T> findOption(const S& s) const 
    {
        for (const auto& item : options) 
            if (item.first == s)
                return item.second;
        return {};
    }
    inline void move(T t, SmallMap<S,T>& other) 
    {
        const size_t size = options.size();
        for (int i = 0; i < size; i++) 
           if (options[i].second == t)
           {
               Element element = options[i];
               other.push(element);
               options.erase(options.begin() + i); 
           }
    }
    inline void remove(T t)
    {
        size_t size = options.size();
        for (int i = 0; i < size; i++) 
           if (options[i].second == t)
               options.erase(options.begin() + i); 
    }
    inline std::vector<S> getStrings() const
    {
        std::vector<std::string> vec;
        vec.reserve(options.size());
        for (const auto& item : options) 
            vec.push_back(item.first);
        return vec;
    }
    inline void push(Element element)
    {
        options.push_back(element);
    }
    inline T end()
    {
        return options.end();
    }
private:
    std::vector<std::pair<S, T>> options;
};

namespace rpt
{

enum class ReportType : uint8_t
{
    Shader,
    Renderpass,
    Pipeline,
    RenderpassInstance,
    RenderCommand,
    DescriptorSetLayout,
    DescriptorSet,
    Attachment,
    PipelineLayout
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
    inline void setSpecInt(int index, int val)
    {
        if (index == 0) specint0 = val;
        if (index == 1) specint1 = val;
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
    enum class Type : uint8_t {swapchain, offscreen}; 
    inline RenderpassReport(std::string n, Type t) :
        name{n}, type{t} {}
    inline RenderpassReport(std::string n, Type t, vk::AttachmentLoadOp loadOp) :
        name{n}, type{t}, loadOp{loadOp} {}
    inline void operator()() const override
    {
        std::cout << "================== Renderpass Report ==================" << std::endl;
        std::cout << "Name:                  " << name << std::endl;
        std::cout << "Type:                  " << typeToString() << std::endl;
        std::cout << "Attachment Load Op:    " << vk::to_string(loadOp) << std::endl;
    }
    inline const std::string getObjectName() const override {return name;}
    inline ReportType getType() const override {return ReportType::Renderpass;}
private:
    const std::string name{"unitilialized"};
    Type type;
    inline std::string typeToString() const
    {
        switch (type)
        {
            case Type::swapchain:
            {
                return "Swapchain";
                break;
            }
            case Type::offscreen:
            {
                return "Offscreen";
                break;
            }
        }
        return "no type";
    }
    vk::AttachmentLoadOp loadOp;
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

class DescriptorSetLayoutReport : public Report
{
public:
    inline DescriptorSetLayoutReport(
            const std::string name, std::vector<vk::DescriptorSetLayoutBinding> bindings) :
   name{name}, bindings{bindings} {}
    inline void operator()() const override
    {
        std::cout << "============= Descriptor Set Layout Report =============" << std::endl;
        std::cout << "Name:                " << name << std::endl;
        auto bcount = bindings.size();
        std::cout << "Binding count:       " << bcount << std::endl;
        for (int i = 0; i < bcount; i++) 
        {
        std::cout << "-----------------------------------" << std::endl;
        std::cout << "Binding " << i << std::endl;
        std::cout << "Descriptor Type:    " << vk::to_string(bindings[i].descriptorType) << std::endl;
        std::cout << "Descriptor Count:   " << bindings[i].descriptorCount << std::endl;
        std::cout << "Shader Stage Flags: " << vk::to_string(bindings[i].stageFlags) << std::endl;
        }
    }
    inline ReportType getType() const override {return ReportType::DescriptorSetLayout;}
    inline const std::string getObjectName() const override {return name;}
private:
    const std::string name;
    const std::vector<vk::DescriptorSetLayoutBinding> bindings;
    inline std::string getDescriptorType(int index) 
    {
        auto binding = bindings.at(index);
        return vk::to_string(binding.descriptorType);
    }
};

class DescriptorSetReport : public Report
{
public:
    enum class Type : uint8_t {frameOwned, rendererOwned};
    inline DescriptorSetReport(
            const std::vector<std::string> descSetLayoutNames, DescriptorSetReport::Type t) :
        descSetLayoutNames{descSetLayoutNames}, type{t} {}
    inline void operator()() const override
    {
        std::cout << "================ Descriptor Set Report ==============" << std::endl;
        auto s = (type == Type::frameOwned) ? "Frame Owned" : "Renderer Owneded";
        std::cout << "Descriptor Set Type: " << s << std::endl;
        std::cout << "Descriptor Set Layout Names: ";
        for (const auto& i : descSetLayoutNames) 
        {
            std::cout << i << ", " << std::endl;
        }
        std::cout << std::endl;
    }
    //not currently useful
    inline const std::string getObjectName() const override {return "0";} 
    inline ReportType getType() const override {return ReportType::DescriptorSet;}
private:
    const std::vector<std::string> descSetLayoutNames;
    Type type;
};

class AttachmentReport : public Report
{
public:
    AttachmentReport(const std::string n, const int w, const int h, vk::ImageUsageFlags f) :
        name{n}, width{w}, height{h}, usageFlags{f} {}
    void operator()() const override
    {
        std::cout << "========= Attachment Report ============" << std::endl;
        std::cout << "Name:        " << name << std::endl;
        std::cout << "Dimensions:  " << width << " " << height << std::endl;
        std::cout << "Usage Flags: " << vk::to_string(usageFlags) << std::endl;
    }
    const std::string getObjectName() const override
    {
        return name;
    }
    ReportType getType() const override
    {
        return ReportType::Attachment;
    }
private:
    const std::string name;
    const int width, height;
    vk::ImageUsageFlags usageFlags;
};

class PipelineLayoutReport : public Report
{
public:
    PipelineLayoutReport(std::string name, std::vector<std::string> dlnames) : name{name}, descriptorSetLayouts{dlnames} {}
    void operator()() const 
    {
        std::cout << "============== Pipeline Layout Report ============== " << std::endl;
        std::cout << "Name:     " << name << std::endl;
        std::cout << "Descriptor set layouts: ";
        for (const auto& i : descriptorSetLayouts) 
        {
            std::cout << i << "   ";
        }
        std::cout << std::endl;
    }
    const std::string getObjectName() const {return name;}
    ReportType getType() const {return ReportType::PipelineLayout;}
private:
    std::string name;
    std::vector<std::string> descriptorSetLayouts;
};

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
    virtual const char* getName() {return "commandBase";}
    inline bool isAvailable() const {return !inUse;}
    template <typename... Args> void set(Args... args) {}
    void reset() {inUse = false;}
    void activate() {inUse = true;}
protected:
    Command() = default;
    bool inUse{false};
};

class RefreshState : public Command
{
public:
    CMD_BASE("refreshState");
    inline void set(State::State* state) {this->state = state;}
private:
    State::State* state{nullptr};
};

class LoadFragShader: public Command
{
public:
    CMD_BASE("loadFragShader");
    inline void set(std::string name) {shaderName = name;}
private:
    std::string shaderName;
};

class LoadVertShader: public Command
{
public:
    CMD_BASE("loadVertShader");
    inline void set(std::string name) {shaderName = name;}
private:
    std::string shaderName;
};

class SetSpecFloat: public Command
{
public:
    CMD_BASE("setSpecFloat");
    inline void set(std::string name, std::string t, float first, float second) {
        shaderName = name; type = t; x = first; y = second;}
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
        shaderName = name; type = t; x = first; y = second;}
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
    inline void set(std::string name, int x, int y, vk::ImageUsageFlags usage)
    {
        assert(x > 0 && y > 0 && "Bad attachment size");
        attachmentName = name;
        this->usage = usage;
        dimensions.setWidth(x);
        dimensions.setHeight(y);
    }
private:
    std::string attachmentName;
    vk::Extent2D dimensions{{500, 500}};
    vk::ImageUsageFlags usage;
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
    inline void set(std::string n, std::vector<vk::DescriptorSetLayoutBinding> b) 
    {
        name = n;
        bindings = b;
    }
private:
    std::string name{"default"};
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

class CreatePipelineLayout : public Command
{
public:
    CMD_BASE("createPipelineLayout")
    inline void set( std::string n, std::vector<std::string> descSetLayoutNames)
    {
        name = n;
        descriptorSetLayoutNames = descSetLayoutNames;
    }
private:
    std::string name{"default"};
    std::vector<std::string> descriptorSetLayoutNames;
};

class PrepareRenderFrames : public Command
{
public:
    CMD_BASE("prepareRenderFrames");
    inline void set(XWindow* window) {this->window = window;}
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
    inline void set(std::string name) {rpassName = name;}
private:
    std::string rpassName;
};

class CreateOffscreenRenderpass : public Command
{
public:
    CMD_BASE("create_offscreen_render_pass");
    inline void set(std::string name, vk::AttachmentLoadOp op) {rpassName = name; loadOp = op;}
private:
    std::string rpassName;
    vk::AttachmentLoadOp loadOp;
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

class UpdateVocab : public Command
{
public:
    CMD_BASE("updatevocab");
private:
};

class CreateFrameDescriptorSets : public Command
{
public:
    CMD_BASE("createFrameDescriptorSets");
    inline void set(std::vector<std::string> layouts) {layoutnames = layouts;}
private:
    std::vector<std::string> layoutnames;
};

class InitFrameUbos : public Command
{
public:
    CMD_BASE("initFrameUbos");
    void set(uint32_t b) {binding = b;}
private:
    uint32_t binding;
};

class UpdateFrameSamplers : public Command
{
public:
    CMD_BASE("updateFrameSamplers");
    void set(uint32_t b) {binding = b;}
private:
    uint32_t binding;
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
using Pool = Pool<T, Command>;

};

using CmdPtr = std::unique_ptr<Command::Command, std::function<void(Command::Command*)>>;

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

class Vocab
{
public:
    Vocab(CommandStack& cmdStack) : cmdStack{cmdStack} {}
    Vocab& operator=(const std::vector<std::string> strings)
    {
        vocab = strings;
        auto cmd = uvPool.request();
        cmdStack.push(std::move(cmd));
        return *this;
    }
    const std::vector<std::string>* getValues() const {return &vocab;}
    inline auto begin() const {return vocab.begin();}
    inline auto end() const {return vocab.end();}

private:
    Command::Pool<Command::UpdateVocab> uvPool{1};
    CommandStack& cmdStack;
    std::vector<std::string> vocab;
};

using namespace rpt;

//TODO: we should create a generic function for parsing the commandline input
//      into arguments that will do type checking for us based on the 
//      argument type
class State
{
public:
    State(EditStack& editStack, CommandStack& cmdStack) : stateEdits{editStack}, cmdStack{cmdStack}, vocab{cmdStack} {}
    virtual void handleEvent(Event* event) = 0;
    void onEnter(Application*);
    void onExit(Application*);
    void onResume(Application* app);
    void onPush(Application* app);
    virtual void refresh(Application* app);
    virtual const char* getName() const = 0;
    virtual std::vector<const Report*> getReports() const {return {};};
protected:
    void pushState(State* state);
    void popState(EditStack* edits);
    void syncReports(State* child);
    void pushCmd(CmdPtr ptr);
    virtual void removeDynamicOps() {}
    virtual void addDynamicOps() { std::cout << "Called Base dynOps" << std::endl;}
    virtual void onResumeExt(Application*) {}
    virtual void onPushExt(Application*) {}
    virtual void onEnterExt(Application*) {}
    virtual void onExitExt(Application*) {}

    inline static Command::Pool<Command::RefreshState> refreshPool{10};
    State* activeChild{nullptr};
    Vocab vocab;
    EditStack& stateEdits;
    CommandStack& cmdStack;
    std::vector<const Report *> reports;
};

class ShaderManager: public State
{
public:
    STATE_BASE("shadermanager");
    ShaderManager(EditStack& es, CommandStack& cs) : State::State{es, cs} {vocab = opMap.getStrings();}
    std::vector<const Report*> getReports() const override
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
    SmallMap<std::string, Option> opMap{
        {"loadfragshaders", Option::loadFragShaders},
        {"loadvertshaders", Option::loadVertShaders},
        {"report", Option::shaderReport},
        {"setspecints", Option::setSpecInts},
        {"setspecfloats", Option::setSpecFloats},
        {"setwindowresolution", Option::setSpecFloats}};

    Mode mode{Mode::null};
    std::map<std::string, std::unique_ptr<ShaderReport>> shaderReports;
};

class AddAttachment: public State
{
public:
    STATE_BASE("addAttachment");
    AddAttachment(EditStack& es, CommandStack& cs) : State::State{es, cs} {vocab = opMap.getStrings();};
    std::vector<const Report*> getReports() const override
    {
        std::vector<const Report*> reports;
        for (const auto& r : attachmentReports) 
        {
            reports.push_back(&r);   
        }
        return reports;
    }
private:
    Command::Pool<Command::AddAttachment> addAttPool{10};
    int width, height;
    enum class Option : uint8_t {setdimensions = 1, fulldescription, addsamplesources};
    enum class Mode : uint8_t {null, setdimensions, fulldescription, addsamplesources};
    SmallMap<std::string, Option> opMap {
        {"setDimensions", Option::setdimensions},
        {"fulldescription", Option::fulldescription},
        {"addSampleSources", Option::addsamplesources}};

    std::vector<rpt::AttachmentReport> attachmentReports;

    Mode mode{Mode::null};
};

class Paint : public State
{
public:
    STATE_BASE("paint")
    Paint(EditStack& es, CommandStack& cs, uint32_t w, uint32_t h) : 
        State::State{es, cs}, width{w}, cwidth{w}, height{h}, cheight{h} 
    {
        cMapX = float(width)/float(cwidth);
        cMapY = float(height)/float(cheight);
        fragInput.xform = glm::scale(glm::mat4(1.), glm::vec3(cMapX, cMapY, 1.0));
        fragInput.xform = toCanvasSpace;
        toCanvasSpace = glm::scale(glm::mat4(1.), glm::vec3(cMapX, cMapY, 1.0)); 
        pivotMatrix = glm::translate(glm::mat4(1.), glm::vec3{-0.5 * cMapX, -0.5 * cMapY, 0.0});

        vocab = opMap.getStrings();
    }
    void onEnterExt(Application* app) override;

    using ReturnType = void;

    ReturnType initial(Event*);
    ReturnType setup(Event*);
    ReturnType paint(Event*);
    ReturnType paint_hm(Event*);

private:
    using MemFunc = ReturnType (Paint::*)(Event*);
    Command::Pool<Command::Render> renderPool{300};
    const uint32_t width, height; //swapchain
    const uint32_t cwidth, cheight; //canvas;
    float cMapX;
    float cMapY;
    FragmentInput fragInput;

    uint32_t paintCommand = 0;
    uint32_t swapOnlyCmd = 1;
    uint32_t paletteCmd = 2;
    uint32_t brushSizeCmd = 3;

    glm::mat4 toCanvasSpace;
    glm::mat4 transMatrix{1.};
    glm::mat4 pivotMatrix;
    glm::mat4 scaleRotMatrix{1.};

    float angleScale{3};
    glm::mat4 transCache{1.};
    glm::mat4 scaleRotCache{1.};
    glm::mat4 scaleCache{1.};
    bool rotateDown{false};
    float scale{1};
    float mouseXCache{0};
    float mouseYCache{0};
    float angle{0};
    float angleInitCache{0};
    float brushSizeCache{1.};
    glm::vec4 mPos{0};
    glm::vec4 transMouseCache{0};

    enum class PressAction : uint8_t 
    {
        Rotate = static_cast<uint8_t>(Key::Alt),
        Color = static_cast<uint8_t>(Key::C),
        BrushSize = static_cast<uint8_t>(Key::B),
        BrushInteractive = static_cast<uint8_t>(Key::Shift_L),
        Exit = static_cast<uint8_t>(Key::Esc),
        Save = static_cast<uint8_t>(Key::S),
        Alpha = static_cast<uint8_t>(Key::A),
        Palette = static_cast<uint8_t>(Key::P)
    };

    enum class InputState : uint8_t
    {
        Paint,
        Rotate,
        Scale, 
        Translate,
        Idle,
        BrushResize
    };

    enum class ReleaseAction : uint8_t 
    {
        Rotate = static_cast<uint8_t>(PressAction::Rotate),
        BrushSize = static_cast<uint8_t>(PressAction::BrushInteractive)
    };

    InputState state{InputState::Idle};

    enum class Option : uint8_t {setup, paint};
    inline static SmallMap< std::string, Option> opMap
    {
        {"setup", Option::setup},
        {"paint", Option::paint},
    };

    enum class Mode : uint8_t {initial, setup, paint};
    static constexpr std::array<MemFunc, 3> functions{&Paint::initial, &Paint::setup, &Paint::paint};

    Mode mode{Mode::initial};
};

class RenderpassManager: public State
{
public:
    STATE_BASE("renderpassManager");
    RenderpassManager(EditStack& es, CommandStack& cs) : State::State{es, cs}  {vocab = opMap.getStrings();}
    std::vector<const Report*> getReports() const override
    {
        std::vector<const Report*> reports;
        for (const auto& item : renderpassReports) 
            reports.push_back(&item);
        return reports;
    }

    enum class Mode : uint8_t {null, createSwapRenderpass, createOffscreenRenderpass};

    using ReturnType = std::optional<Mode>;

    ReturnType initial(Event*);
    ReturnType createSwapRenderpass(Event*);
    ReturnType createOffscreenRenderpass(Event*);

private:    
    Command::Pool<Command::CreateSwapchainRenderpass> csrPool;
    Command::Pool<Command::CreateOffscreenRenderpass> corPool;
    enum class Option : uint8_t {createSwapRenderpass, createOffscreenRenderpass, report};
    SmallMap<std::string, Option> opMap
    {
        {"create_offscreen_renderpass", Option::createOffscreenRenderpass},
        {"create_swap_renderpass", Option::createSwapRenderpass},
        {"report", Option::report}
    };

    using MemFunc = ReturnType (RenderpassManager::*)(Event*);
    SmallMap<Mode, MemFunc> modeToFunc
    {
        {Mode::null, &RenderpassManager::initial},
        {Mode::createSwapRenderpass, &RenderpassManager::createSwapRenderpass},
        {Mode::createOffscreenRenderpass, &RenderpassManager::createOffscreenRenderpass}
    };
    std::vector<RenderpassReport> renderpassReports;
    Mode mode{Mode::null};
};

class PipelineManager : public State
{
public:
    STATE_BASE("pipelineManager");
    PipelineManager(EditStack& es, CommandStack& cs) : State::State{es, cs}  {vocab = opMap.getStrings();}
    inline void setCGPVocab(std::vector<std::string> v) { cgpVocab = v;}
    std::vector<const Report*> getReports() const override
    {
        std::vector<const Report*> reports;
        for (const auto& item : gpReports) 
            reports.push_back(item.second.get());
        for (const auto& i : pipelineLayoutReports) 
        {
            reports.push_back(&i);   
        }
        return reports;
    }
    void createPipelineLayout(Event*);
    void createGraphicsPipeline(Event*);
    void base(Event*);
private:
    Command::Pool<Command::CreateGraphicsPipeline> cgpPool{5};
    Command::Pool<Command::CreatePipelineLayout> cpplPool{1};
    enum class Option : uint8_t {createGraphicsPipeline, report, createPipelineLayout};
    enum class Mode : uint8_t {null, createGraphicsPipeline, createPipelineLayout};
    SmallMap<std::string, Option> opMap
    {
        {"create_graphics_pipeline", Option::createGraphicsPipeline},
        {"report", Option::report },
        {"create_pipeline_layout", Option::createPipelineLayout}
    };
    SmallMap<Mode, void(PipelineManager::*)(Event*)> modeToFunc
    {
        {Mode::null, &PipelineManager::base},
        {Mode::createGraphicsPipeline, &PipelineManager::createGraphicsPipeline},
        {Mode::createPipelineLayout, &PipelineManager::createPipelineLayout}
    };

    Mode mode{Mode::null};
    std::map<std::string, std::unique_ptr<GraphicsPipelineReport>> gpReports;
    std::vector<PipelineLayoutReport> pipelineLayoutReports;

    std::vector<std::string> cgpVocab;
};

class CreateDescriptorSetLayout : public State
{
public:
    STATE_BASE("createDescriptorSetLayout")
    CreateDescriptorSetLayout(EditStack& es, CommandStack& cs) : State::State{es, cs}  {vocab = opMap.getStrings();}
    void initial(Event*);
    void enterBindings(Event*);
    void createLayout(Event*);
    std::vector<const Report*> getReports() const override
    {
        std::vector<const Report*> reports;
        for (const auto& item : descSetLayoutReports) 
        {
            reports.push_back(&item);
        }
        return reports;
    }
private:
    using MemFunc = void (CreateDescriptorSetLayout::*)(Event*);
    Command::Pool<Command::CreateDescriptorSetLayout> cdslPool{1};
    std::vector<vk::DescriptorSetLayoutBinding> pendingBindings;
    std::vector<DescriptorSetLayoutReport> descSetLayoutReports;
    enum class Stage : uint8_t {descriptorTypeEntry, descriptorCountEntry, shaderStageEntry};
    enum class Option : uint8_t {enterBindings = 1, createDescriptorSetLayout};
    enum class Mode : uint8_t {null, enterBindings, createLayout};
    Mode mode{Mode::null};
    uint8_t curStage{0};
    uint8_t bindingCount{0};
    SmallMap<std::string, Option> opMap{
            {"enterbindings", Option::enterBindings},
            {"createdescriptorsetlayout", Option::createDescriptorSetLayout}};
    SmallMap<std::string, vk::DescriptorType> descTypeMap{
            {"uniformbuffer", vk::DescriptorType::eUniformBuffer},
            {"combinedImageSampler", vk::DescriptorType::eCombinedImageSampler}};
    SmallMap<std::string, vk::ShaderStageFlagBits> shaderStageMap{
            {"fragmentShader", vk::ShaderStageFlagBits::eFragment}};
    SmallMap<Mode, MemFunc> funcMap{
            {Mode::null, &CreateDescriptorSetLayout::initial},
            {Mode::enterBindings, &CreateDescriptorSetLayout::enterBindings},
            {Mode::createLayout, &CreateDescriptorSetLayout::createLayout}};
    SmallMap<vk::DescriptorType, std::string> descToString
    {
        {vk::DescriptorType::eUniformBuffer, "Uniform Buffer"},
        {vk::DescriptorType::eCombinedImageSampler, "Combined Image Sampler"}
    };
};

class DescriptorManager : public State
{
public:
    STATE_BASE("descriptorManager");
    DescriptorManager(EditStack& es, CommandStack& cs) : State::State{es, cs}, cdsl{es, cs}  {vocab = opMap.getStrings();}
    std::vector<const Report*> getReports() const override
    {
        std::vector<const Report*> reps = reports;
        for (const auto& r : descSetReports) 
        {
            reps.push_back(&r);
        }
        return reps;
    }

    void base(Event*);
    void createFrameDescriptorSets(Event*);
    void initFrameUbos(Event*);
    void updateFrameSamplers(Event*);
private:
    using MemFunc = void (DescriptorManager::*)(Event*);
    Command::Pool<Command::CreateFrameDescriptorSets> cfdsPool{1};
    Command::Pool<Command::InitFrameUbos> ifuPool{1};
    Command::Pool<Command::UpdateFrameSamplers> ufsPool{1};
    CreateDescriptorSetLayout cdsl;
    enum class Option {initFrameUbos, updateFrameSamplers, createDescriptorSetLayout, createFrameDescriptorSets, printReports};
    enum class Mode {null, createFrameDescriptorSets, initFrameUbos, updateFrameSamplers};
    SmallMap<std::string, Option> opMap{
            {"create_descriptor_set_layout", Option::createDescriptorSetLayout},
            {"create_frame_descriptor_sets", Option::createFrameDescriptorSets},
            {"print_reports", Option::printReports},
            {"initframeubos", Option::initFrameUbos},
            {"updateframesamplers", Option::updateFrameSamplers},
    };
    SmallMap<std::string, Option> resMap{};
    SmallMap<Mode, MemFunc> funcMap
    {
        {Mode::null, &DescriptorManager::base},
        {Mode::createFrameDescriptorSets, &DescriptorManager::createFrameDescriptorSets},
        {Mode::updateFrameSamplers, &DescriptorManager::updateFrameSamplers},
        {Mode::initFrameUbos, &DescriptorManager::initFrameUbos}
    };
    static constexpr std::array<Option, 2> dynamicOptions{Option::createDescriptorSetLayout, Option::createFrameDescriptorSets};
    inline void removeDynamicOps() override { for (const auto& op : dynamicOptions) opMap.move(op, resMap); vocab = opMap.getStrings();}
    inline void addDynamicOps() override { for (const auto& op : dynamicOptions) resMap.move(op, opMap); vocab = opMap.getStrings();}

    std::vector<DescriptorSetReport> descSetReports;

    Mode mode{Mode::null};
};

class RendererManager : public State
{
public:
    inline RendererManager(XWindow& the_window, EditStack& es, CommandStack& cs) : 
        window{the_window}, 
        State::State{es, cs},
        rpassManager{es, cs},
        shaderManager{es, cs},
        pipelineManager{es, cs},
        addAttachState{es, cs},
        descriptorManager{es, cs}
        {vocab = opMap.getStrings();}
    STATE_BASE("rendererManager");
    void onPushExt(Application* app) override;

    void base(Event*);
    void createRPI(Event*);
    void render(Event*);
    void recordRenderCmd(Event*);
private:
    using MemFunc = void (RendererManager::*)(Event*);
    Command::Pool<Command::PrepareRenderFrames> prfPool{1};
    Command::Pool<Command::OpenWindow> owPool{1};
    Command::Pool<Command::CreateRenderpassInstance> criPool{5};
    Command::Pool<Command::RecordRenderCommand> rrcPool{5};
    Command::Pool<Command::Render> renderPool{5};
    RenderpassManager rpassManager;
    PipelineManager pipelineManager;
    ShaderManager shaderManager;
    AddAttachment addAttachState;
    DescriptorManager descriptorManager;

    enum class Option : uint8_t {render = 1, descriptionManager, recordRenderCmd, rpiReport, printReports, createRPI, shaderManager, openWindow, addattachState, prepRenderFrames, rpassManager, pipelineManager, all};
    enum class Mode : uint8_t {null, createRPI, render, recordRenderCmd};
    SmallMap<std::string, Option> opMap
    {
        {"openWindow", Option::openWindow},
        {"addAttachState", Option::addattachState},
        {"shaderManager", Option::shaderManager},
        {"all", Option::all},
        {"printReports", Option::printReports},
        {"rpiReports", Option::rpiReport},
    };

    SmallMap<std::string, Option> resMap{
    };

    SmallMap<std::string, Option> dependentMap
    {
        {"descriptionmanager", Option::descriptionManager},
        {"rpassManager", Option::rpassManager},
        {"pipelineManager", Option::pipelineManager},
        {"prepRenderFrames", Option::prepRenderFrames},
        {"render", Option::render},
        {"recordRenderCmd", Option::recordRenderCmd},
        {"createRenderpassInstance", Option::createRPI},
    };

    SmallMap<Mode, MemFunc> modeToFunc
    {
        {Mode::null, &RendererManager::base},
        {Mode::createRPI, &RendererManager::createRPI},
        {Mode::recordRenderCmd, &RendererManager::recordRenderCmd},
        {Mode::render, &RendererManager::render}
    };

    static constexpr std::array<Option, 9> dynamicOptions{Option::descriptionManager, Option::shaderManager, Option::addattachState, Option::rpassManager, Option::pipelineManager, Option::all, Option::createRPI, Option::recordRenderCmd, Option::render};
    inline void removeDynamicOps() override { for (const auto& op : dynamicOptions) opMap.move(op, resMap); vocab = opMap.getStrings();}
    inline void addDynamicOps() override { for (const auto& op : dynamicOptions) resMap.move(op, opMap); vocab = opMap.getStrings();}

    Mode mode{Mode::null};
    XWindow& window;

    std::vector<std::unique_ptr<RenderpassInstanceReport>> rpiReports;
    std::vector<std::unique_ptr<RenderCommandReport>> rcReports;
};

class Director: public State
{
public:
    STATE_BASE("director");
    Director(XWindow& window, const StateStack& ss, EditStack& es, CommandStack& cs) : 
        initRenState{window, es, cs}, 
        stateStack{ss},
        State::State{es, cs},
        paint{es, cs, window.getWidth(), window.getHeight()}
    {vocab = opMap.getStrings();}
    void onPushExt(Application*) override;
    void onResumeExt(Application*) override;
private:
    RendererManager initRenState;
    Paint paint;
    enum class Option : uint8_t {initRenState = 1, paint, printStack};
    SmallMap<std::string, Option> opMap
    {
        {"rendererManager", Option::initRenState},
        {"printStack", Option::printStack},
        {"painter", Option::paint}
    };
    SmallMap<std::string, Option> resMap
    {
    };

    static constexpr std::array<Option, 2> dynamicOptions{Option::paint, Option::initRenState};
    inline void removeDynamicOps() override { for (const auto& op : dynamicOptions) opMap.move(op, resMap);}
    inline void addDynamicOps() override { for (const auto& op : dynamicOptions) resMap.move(op,opMap);}

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
    Application(uint16_t w, uint16_t h, const std::string logfile, int eventPops = 0);
    void setVocabulary(State::Vocab vocab);
    void run();
    void popState();
    void pushState(State::State* const);
    void createPipelineLayout();
    void loadDefaultShaders();
    void createDefaultRenderPasses();
    void initUBO();

    void readEvents(std::ifstream&, int eventPops);
    void recordEvent(Event* event, std::ofstream& os);

    Context context;
    XWindow window;
    Renderer renderer;
    EventHandler ev;
    vk::Extent2D offscreenDim;
    vk::Extent2D swapDim;;
    std::ofstream os;
    std::ifstream is;
   
    FragmentInput fragInput;
    std::vector<const mm::Image*> sampledImages;

private:
    std::string readlog;
    std::string writelog{"eventlog"};
    bool recordevents{true};
    bool readevents{false};
    
    StateStack stateStack;
    EditStack stateEdits;
    CommandStack cmdStack;

    State::Director dirState;
};
