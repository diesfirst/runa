#include <command/rendercommands.hpp>
#include <application.hpp>
#include <state/report.hpp>

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
    app->renderer.loadFragShader(state::SHADER_DIR + shaderName, shaderName);
    success();
}

state::Report* LoadFragShader::makeReport() const
{
    auto report = new state::ShaderReport(shaderName, "f", 0, 0, 0, 0);
    return report;
}

void LoadVertShader::execute(Application* app)
{
    app->renderer.loadVertShader(state::SHADER_DIR + shaderName, shaderName);
    success();
}

state::Report* LoadVertShader::makeReport() const
{
    auto report = new state::ShaderReport(shaderName, "v", 0, 0, 0, 0);
    return report;
}

void SetSpecFloat::execute(Application* app)
{
    if (type == "f")
    {
        render::FragShader& fs = app->renderer.fragShaderAt(shaderName);
        fs.setWindowResolution(x, y);
    }
    if (type == "v")
    {
        render::VertShader& vs = app->renderer.vertShaderAt(shaderName);
        vs.setWindowResolution(x, y);
    }
    success();
}

state::Report* SetSpecFloat::makeReport() const
{
    auto report = new state::ShaderReport(shaderName, type.c_str(), 0, 0, x, y);
    report->setUsage(state::ReportUsage::merge);
    return report;
}

void SetSpecInt::execute(Application* app)
{
    if (type == "f")
    {
        render::FragShader& fs = app->renderer.fragShaderAt(shaderName);
        fs.specData.integer0 = x;
        fs.specData.integer1 = y;
    }
    if (type == "v")
    {
        render::VertShader& vs = app->renderer.vertShaderAt(shaderName);
        vs.specData.integer0 = x;
        vs.specData.integer1 = y;
    }
    success();
}

state::Report* SetSpecInt::makeReport() const
{
    auto report = new state::ShaderReport(shaderName, type.c_str(), x, y, 0, 0);
    report->setUsage(state::ReportUsage::merge);
    return report;
}

void CreateFrameDescriptorSets::execute(Application* app)
{
    app->renderer.createFrameDescriptorSets(layoutnames);
    success();
}

void CreateDescriptorSetLayout::execute(Application* app)
{
    app->renderer.createDescriptorSetLayout(name, bindings);
    success();
}

void InitFrameUbos::execute(Application* app)
{
    app->renderer.initFrameUBOs(sizeof(app->fragInput), binding);
    success();
}

void UpdateFrameSamplers::execute(Application* app)
{
    app->renderer.updateFrameSamplers(app->sampledImages, binding);
    success();
}

void CreateSwapchainRenderpass::execute(Application* app)
{
    auto& rpass = app->renderer.createRenderPass(rpassName);
    app->renderer.prepareAsSwapchainPass(rpass);
    success();
}

void CreateOffscreenRenderpass::execute(Application* app)
{
    auto& rpass = app->renderer.createRenderPass(rpassName);
    app->renderer.prepareAsOffscreenPass(rpass, loadOp);
    success();
}


//void AddAttachment::execute(Application* app)
//{
//        auto& attachment = app->renderer.createAttachment(attachmentName, dimensions, usage);
//        if (usage & vk::ImageUsageFlagBits::eSampled)
//            app->sampledImages.push_back(&attachment.getImage(0));
//}
//
//void OpenWindow::execute(Application* app)
//{
//    app->window.open();
//}


void CreateGraphicsPipeline::execute(Application* app)
{
    auto& pipeline = app->renderer.createGraphicsPipeline(
            name, pipelineLayout,
            vertshader, fragshader,
            renderpass, renderArea, is3d);
    pipeline.create();
    success();
}

void CreateRenderpassInstance::execute(Application* app)
{
    app->renderer.addRenderPassInstance(attachment, renderpass, pipeline);
    success();
}

void RecordRenderCommand::execute(Application* app)
{
    app->renderer.recordRenderCommands(cmdBufferId, renderpassInstances);
    success();
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


}; // namespace command

}; // namespace sword
