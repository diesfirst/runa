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


}; // namespace command

}; // namespace sword
