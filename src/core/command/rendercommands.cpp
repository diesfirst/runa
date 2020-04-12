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

}; // namespace command

}; // namespace sword
