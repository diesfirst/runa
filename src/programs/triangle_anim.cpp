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

struct MyUbo
{
    glm::mat4 xform{1.0}; //identity
};

int main(int argc, char *argv[])
{
    Context context;
    XWindow window{WIDTH, HEIGHT};
    window.open();

    Renderer renderer{context, window};

    //load quad shader
    auto& triShader = renderer.loadVertShader(std::string(SHADER_DIR) + "tri_xform.spv", "tri");
    auto& fragShader = renderer.loadFragShader(std::string(SHADER_DIR) + "red.spv", "tri");

    vk::DescriptorSetLayoutBinding uboBinding;
    uboBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    uboBinding.setBinding(0);
    uboBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    uboBinding.setDescriptorCount(1);

    auto descSetLayout = renderer.createDescriptorSetLayout("dLayout", {uboBinding});
    auto pipeLayout = renderer.createPipelineLayout("pLayout", {descSetLayout});

    renderer.createFrameDescriptorSets({descSetLayout});
    
    //make renderpass
    auto& renderPass = renderer.createRenderPass("rp", true);
    renderer.prepareAsSwapchainPass(renderPass);

    auto& pipeline = renderer.createGraphicsPipeline("pipe", pipeLayout, triShader, fragShader, renderPass, false);
    pipeline.rasterizationState.setPolygonMode(vk::PolygonMode::eFill);
    pipeline.create();

    renderer.addFramebuffer(TargetType::swapchain, renderPass, pipeline);

    renderer.initFrameUBOs(sizeof(MyUbo), 0);

    std::cout << "UBO SIZE: " << sizeof(MyUbo) << std::endl;

    renderer.recordRenderCommands(0, {0});

    MyUbo myUbo;
    renderer.bindUboData(static_cast<void*>(&myUbo), sizeof(MyUbo), 0);

    EventHandler eventHandler{window};

    while (true)
    {
        auto& input = eventHandler.fetchUserInput(true);
        float scaleX = (float)input.mouseX / WIDTH;
        float scaleY = (float)input.mouseY / HEIGHT;
        std::cout << scaleX << std::endl;
        myUbo.xform = glm::scale(glm::mat4(1.0), {scaleX, scaleY, 1.0});
        renderer.render(0, true);
    }

    sleep(4);
}

