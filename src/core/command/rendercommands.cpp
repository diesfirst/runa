#include <command/rendercommands.hpp>
#include <application.hpp>

namespace sword
{

namespace command
{

void PrepareRenderFrames::execute(Application* app)
{
    app->renderer.prepareRenderFrames(app->window);
}

void LoadFragShader::execute(Application* app)
{
    app->renderer.loadFragShader(state::SHADER_DIR + shaderName, shaderName);
}

void LoadVertShader::execute(Application* app)
{
    app->renderer.loadVertShader(state::SHADER_DIR + shaderName, shaderName);
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
}

void CreateFrameDescriptorSets::execute(Application* app)
{
    app->renderer.createFrameDescriptorSets(layoutnames);
}

void CreateDescriptorSetLayout::execute(Application* app)
{
    app->renderer.createDescriptorSetLayout(name, bindings);
}

void InitFrameUbos::execute(Application* app)
{
    app->renderer.initFrameUBOs(sizeof(app->fragInput), binding);
}

void UpdateFrameSamplers::execute(Application* app)
{
    app->renderer.updateFrameSamplers(app->sampledImages, binding);
}

void CreateSwapchainRenderpass::execute(Application* app)
{
    auto& rpass = app->renderer.createRenderPass(rpassName);
    app->renderer.prepareAsSwapchainPass(rpass);
}

void CreateOffscreenRenderpass::execute(Application* app)
{
    auto& rpass = app->renderer.createRenderPass(rpassName);
    app->renderer.prepareAsOffscreenPass(rpass, loadOp);
}



}; // namespace command

}; // namespace sword
