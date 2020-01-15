#include "../core/window.hpp"
#include "../core/renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "../core/util.hpp"
#include <unistd.h>
#include "../core/event.hpp"

constexpr uint32_t N_FRAMES = 10000000;
constexpr uint32_t WIDTH = 666;
constexpr uint32_t HEIGHT = 888;

constexpr char SHADER_DIR[] = "build/shaders/";
Timer myTimer;

int main(int argc, char *argv[])
{
    Context context;
    XWindow window(WIDTH, HEIGHT);
    window.open();

    Renderer renderer(context, window);

    //load quad shader
    auto& triShader = renderer.loadVertShader(std::string(SHADER_DIR) + "fullscreen_tri.spv", "tri");
    auto& fragShader = renderer.loadFragShader(std::string(SHADER_DIR) + "red.spv", "tri");

    auto descSetLayout = renderer.createDescriptorSetLayout("dLayout", {});
    auto pipeLayout = renderer.createPipelineLayout("pLayout", {descSetLayout});

    renderer.createFrameDescriptorSets({descSetLayout});
    
    //make renderpass
    auto& renderPass = renderer.createRenderPass("rp", true);
    renderer.prepareAsSwapchainPass(renderPass);

    auto& pipeline = renderer.createGraphicsPipeline("pipe", pipeLayout, triShader, fragShader, renderPass, false);
    pipeline.rasterizationState.setPolygonMode(vk::PolygonMode::eFill);
    pipeline.create();

    renderer.addFramebuffer(TargetType::swapchain, renderPass, pipeline);

    renderer.recordRenderCommands(0, {0});

    renderer.render(0, false);

    sleep(4);
}

