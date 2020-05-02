#ifndef REPORT_HPP_
#define REPORT_HPP_

#include <string>
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <util/enum.hpp>

namespace sword
{

namespace state
{

enum class ReportUsage : uint8_t
{
    add,
    merge,
    remove
};

enum class ReportType : uint8_t
{
    Shader,
    RenderPass,
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
    virtual ~Report() = default;

    void setUsage(ReportUsage usage) { this->usage = usage; }
    ReportUsage getUsage() const {return usage;}
private:
    ReportUsage usage{ReportUsage::add};
};

class ShaderReport final : public Report
{
public:
    ShaderReport(std::string n, ShaderType t, int i0, int i1, float f0, float f1) :
        name{n}, type{t}, specint0{i0}, specint1{i1}, specfloat0{f0}, specfloat1{f1} {}
    void operator()() const  override
    {
        std::cout << "================== Shader Report ==================" << std::endl;
        std::cout << "Name:                  " << name << std::endl;
        std::cout << "Type:                  " << string_type() << std::endl;
        std::cout << "Spec Constant Int 0:   " << specint0 << std::endl;
        std::cout << "Spec Constant Int 1:   " << specint1 << std::endl;
        std::cout << "Spec Constant Float 0: " << specfloat0 << std::endl;
        std::cout << "Spec Constant Float 1: " << specfloat1 << std::endl;
    }
    const std::string getObjectName() const override {return name;};
    ReportType getType() const override {return ReportType::Shader;}
    void setSpecFloat(int index, float val)
    {
        if (index == 0) specfloat0 = val;
        if (index == 1) specfloat1 = val;
    }
    void setSpecInt(int index, int val)
    {
        if (index == 0) specint0 = val;
        if (index == 1) specint1 = val;
    }
private:
    constexpr std::string_view string_type() const
    {
        switch (type)
        {
            case ShaderType::frag: return "Fragment";
            case ShaderType::vert: return "Vertex";
        }
        return "";
    }
    const std::string name{"unitilialized"};
    const char* type_string{"uninilialized"};
    ShaderType type;
    int specint0{0};
    int specint1{0};
    float specfloat0{0};
    float specfloat1{0};
};

class GraphicsPipelineReport : public Report
{
public:
    GraphicsPipelineReport(
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
    void operator()() const override
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
    const std::string getObjectName() const override {return name;}
    ReportType getType() const override {return ReportType::Pipeline;}
    const std::string_view getFragShader() const { return fragshader; }
    const std::string_view getVertShader() const { return vertshader; }
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

class RenderPassReport : public Report
{
public:
    enum class Type : uint8_t {swapchain, offscreen}; 
    inline RenderPassReport(std::string n, Type t) :
        name{n}, type{t} {}
    inline RenderPassReport(std::string n, Type t, vk::AttachmentLoadOp loadOp) :
        name{n}, type{t}, loadOp{loadOp} {}
    inline void operator()() const override
    {
        std::cout << "================== RenderPass Report ==================" << std::endl;
        std::cout << "Name:                  " << name << std::endl;
        std::cout << "Type:                  " << typeToString() << std::endl;
        std::cout << "Attachment Load Op:    " << vk::to_string(loadOp) << std::endl;
    }
    inline const std::string getObjectName() const override {return name;}
    inline ReportType getType() const override {return ReportType::RenderPass;}
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

class RenderLayerReport : public Report
{
public:
    RenderLayerReport(
            std::string attachName,
            std::string rpassName,
            std::string pipeName,
            int id) :
        attachmentName{attachName},
        renderpassName{rpassName},
        pipelineName{pipeName},
        id{id} {}
    void operator()() const override
    {
        std::cout << "============== Renderpass Instance Report =============" << std::endl;
        std::cout << "Index:           " << id << std::endl;
        std::cout << "Attachment Name: " << attachmentName << std::endl;
        std::cout << "Renderpass Name: " << renderpassName << std::endl;
        std::cout << "Pipeline Name:   " << pipelineName << std::endl;
    }
    ReportType getType() const override {return ReportType::RenderpassInstance;}
    const std::string getObjectName() const override {return std::to_string(id);}
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


}; //state

}; //sword

#endif /* ifndef REPORT_HPP_ */
