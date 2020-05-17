#include <command/rendercommands.hpp>
#include <application.hpp>
#include <state/report.hpp>
#include <util/defs.hpp>
#include <render/attachment.hpp>

namespace sword
{

namespace command
{

void PrepareRenderFrames::execute(Application* app)
{
    app->renderer.prepareRenderFrames(app->window);
    success();
}

void LoadFragShader::execute(Application* app)
{
    app->renderer.loadFragShader(SHADER_DIR + std::string("/") + shaderName, shaderName);
    success();
}

state::Report* LoadFragShader::makeReport() const
{
    return new state::ShaderReport(shaderName, ShaderType::frag, 0, 0, 0, 0);
}

void LoadVertShader::execute(Application* app)
{
    app->renderer.loadVertShader(SHADER_DIR + std::string("/") + shaderName, shaderName);
    success();
}

state::Report* LoadVertShader::makeReport() const
{
    return new state::ShaderReport(shaderName, ShaderType::vert, 0, 0, 0, 0);
}

void SetSpecFloat::execute(Application* app)
{
    if (type == ShaderType::frag)
    {
        render::FragShader& fs = app->renderer.fragShaderAt(shaderName);
        fs.setWindowResolution(x, y);
    }
    if (type == ShaderType::vert)
    {
        render::VertShader& vs = app->renderer.vertShaderAt(shaderName);
        vs.setWindowResolution(x, y);
    }
    success();
}

state::Report* SetSpecFloat::makeReport() const
{
    auto report = new state::ShaderReport(shaderName, type, 0, 0, x, y);
    report->setUsage(state::ReportUsage::merge);
    return report;
}

void SetSpecInt::execute(Application* app)
{
    if (type == ShaderType::frag)
    {
        render::FragShader& fs = app->renderer.fragShaderAt(shaderName);
        fs.specData.integer0 = x;
        fs.specData.integer1 = y;
    }
    if (type == ShaderType::vert)
    {
        render::VertShader& vs = app->renderer.vertShaderAt(shaderName);
        vs.specData.integer0 = x;
        vs.specData.integer1 = y;
    }
    success();
}

state::Report* SetSpecInt::makeReport() const
{
    auto report = new state::ShaderReport(shaderName, type, x, y, 0, 0);
    report->setUsage(state::ReportUsage::merge);
    return report;
}

void CreateFrameDescriptorSets::execute(Application* app)
{
    app->renderer.createFrameDescriptorSets(layoutnames);
    success();
}

state::Report* CreateFrameDescriptorSets::makeReport() const
{
    return new state::DescriptorSetReport(layoutnames, state::DescriptorSetReport::Type::frameOwned);
}

void CreateDescriptorSetLayout::execute(Application* app)
{
    app->renderer.createDescriptorSetLayout(name, bindings);
    success();
}

state::Report* CreateDescriptorSetLayout::makeReport() const
{
    return new state::DescriptorSetLayoutReport(name, bindings);
}

void InitFrameUbos::execute(Application* app)
{
    app->renderer.initFrameUBOs(size, binding);
    success();
}

void UpdateFrameSamplers::execute(Application* app)
{
    if (!attachmentNames.empty())
    {
        app->renderer.updateFrameSamplers(attachmentNames, binding);
        success();
    }
}

void CreateSwapchainRenderpass::execute(Application* app)
{
    auto& rpass = app->renderer.createRenderPass(rpassName);
    app->renderer.prepareAsSwapchainPass(rpass);
    success();
}

state::Report* CreateSwapchainRenderpass::makeReport() const
{
    return new state::RenderPassReport(rpassName, state::RenderPassReport::Type::swapchain);
}

void CreateOffscreenRenderpass::execute(Application* app)
{
    auto& rpass = app->renderer.createRenderPass(rpassName);
    app->renderer.prepareAsOffscreenPass(rpass, loadOp);
    success();
}

state::Report* CreateOffscreenRenderpass::makeReport() const
{
    return new state::RenderPassReport(rpassName, state::RenderPassReport::Type::offscreen, loadOp);
}


void AddAttachment::execute(Application* app)
{
        auto& attachment = app->renderer.createAttachment(attachmentName, dimensions, usage);
        if (usage & vk::ImageUsageFlagBits::eSampled)
            app->sampledImages.push_back(&attachment.getImage(0));
        success();
}

void OpenWindow::execute(Application* app)
{
    app->window.open();
    success();
}


void CreateGraphicsPipeline::execute(Application* app)
{
    bool res;
    if (report)
    {
        std::cout << "commaned::CreateGraphicsPipeline: Found report. Name is: " << report->getObjectName() << '\n';
        res = app->renderer.recreateGraphicsPipeline(report->getObjectName());
    }
    else 
        res = app->renderer.createGraphicsPipeline(
            name, pipelineLayout,
            vertshader, fragshader,
            renderpass, renderArea, is3d);
    if (res)
        success();
}

state::Report* CreateGraphicsPipeline::makeReport() const
{
    if (report)
    {
        return report;
    }
    return new state::GraphicsPipelineReport(name, pipelineLayout, vertshader, fragshader, renderpass, 
            renderArea.offset.x, renderArea.offset.y, renderArea.extent.width, renderArea.extent.height, is3d);
}

void CreateRenderLayer::execute(Application* app)
{
    app->renderer.addRenderLayer(attachment, renderpass, pipeline);
    success();
}

state::Report* CreateRenderLayer::makeReport() const
{

    auto report = new state::RenderLayerReport(attachment, renderpass, pipeline, id);
    id++;
    return report;
}

void RecordRenderCommand::execute(Application* app)
{
    app->renderer.recordRenderCommands(cmdBufferId, renderLayers);
    success();
}

state::Report* RecordRenderCommand::makeReport() const
{
    if (report)
    {
        return report;
    }
    return new state::RenderCommandReport(cmdBufferId, renderLayers);
}

void Render::execute(Application* app)
{
    app->renderer.render(renderCommandId, updateUBO);
    success();
}


void CreatePipelineLayout::execute(Application* app)
{
    app->renderer.createPipelineLayout(name, descriptorSetLayoutNames);
    success();
}

state::Report* CreatePipelineLayout::makeReport() const
{
    return new state::PipelineLayoutReport(name, descriptorSetLayoutNames);
}

void BindUboData::execute(Application* app)
{
    app->renderer.bindUboData(address, size, 0);
}


}; // namespace command

}; // namespace sword
