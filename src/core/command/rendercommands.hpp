#ifndef RENDERCOMMANDS_HPP_
#define RENDERCOMMANDS_HPP_

// imp: command/rendercommands.cpp

#include "command.hpp"
#include <vulkan/vulkan.hpp>

namespace sword
{

namespace command
{

class LoadFragShader: public Command
{
public:
    CMD_BASE("loadFragShader");
    void set(std::string name) {shaderName = name;}
    state::Report* makeReport() const override;
private:
    std::string shaderName;
};

class LoadVertShader: public Command
{
public:
    CMD_BASE("loadVertShader");
    void set(std::string name) {shaderName = name;}
    state::Report* makeReport() const override;
private:
    std::string shaderName;
};

class SetSpecFloat: public Command
{
public:
    CMD_BASE("setSpecFloat");
    void set(std::string name, std::string t, float first, float second) {
        shaderName = name; type = t; x = first; y = second;}
    state::Report* makeReport() const override;
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
    state::Report* makeReport() const override;
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
    state::Report* makeReport() const override;
private:
    std::string name{"default"};
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

class CreatePipelineLayout : public Command
{
public:
    CMD_BASE("createPipelineLayout")
    void set( std::string n, std::vector<std::string> descSetLayoutNames)
    {
        name = n;
        descriptorSetLayoutNames = descSetLayoutNames;
    }
    state::Report* makeReport() const override;
private:
    std::string name{"default"};
    std::vector<std::string> descriptorSetLayoutNames;
};

class PrepareRenderFrames : public Command
{
public:
    CMD_BASE("prepareRenderFrames");
private:
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
    state::Report* makeReport() const override;
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
    void set(std::string name) {rpassName = name;}
    state::Report* makeReport() const override;
private:
    std::string rpassName;
};

class CreateOffscreenRenderpass : public Command
{
public:
    CMD_BASE("create_offscreen_render_pass");
    void set(std::string name, vk::AttachmentLoadOp op) {rpassName = name; loadOp = op;}
    state::Report* makeReport() const override;
private:
    std::string rpassName;
    vk::AttachmentLoadOp loadOp;
};

class CreateRenderLayer : public Command
{
public:
    CMD_BASE("createRenderLayer");
    void set(
            std::string attachName,
            std::string renderpassName,
            std::string pipelineName) 
    {
        attachment = attachName;
        renderpass = renderpassName;
        pipeline = pipelineName;
    }
    state::Report* makeReport() const override;
private:
    std::string attachment;
    std::string renderpass;
    std::string pipeline;
    inline static int id{0};
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

class CreateFrameDescriptorSets : public Command
{
public:
    CMD_BASE("createFrameDescriptorSets");
    inline void set(std::vector<std::string> layouts) {layoutnames = layouts;}
    state::Report* makeReport() const override;
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


}; //command
}; //sword

#endif /* ifndef RENDERCOMMANDS_HPP_ */
