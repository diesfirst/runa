#include <application.hpp>
#include <fstream>
#include <thread>
#include <regex>

int main(int argc, const char *argv[])
{
    assert(argc == 3 && "Must provide two arguments: [1]: A glsl shader code string. [2]: A name for the output png");
    using strvec = std::vector<std::string>;

    auto glslstr = "#version 450\n" + std::string(argv[1]);
    auto path = std::string(argv[2]);

    std::cout << "Path:" << '\n';
    std::cout << path << '\n';

    //float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    //std::this_thread::sleep_for(std::chrono::milliseconds(200) * r); //need to stagger the creation of vk device

    sword::Application app{false};
    auto& cp = app.cmdPools;

    app.pushCmd(cp.addAttachment.request("canvas", 1024, 1024, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc ));
    app.pushCmd(cp.loadVertShader.request("fullscreen_tri.spv"));
    app.pushCmd(cp.compileShaderCode.request(glslstr, "rings", sword::ShaderType::frag));
    app.pushCmd(cp.prepareRenderFrames.request());
    app.pushCmd(cp.createOffscreenRenderpass.request("rpass", vk::AttachmentLoadOp::eClear));
    app.pushCmd(cp.createDescriptorSetLayout.request("descriptorlayout", std::vector<vk::DescriptorSetLayoutBinding>{}));
    app.pushCmd(cp.createPipelineLayout.request("layout", strvec{"descriptorlayout"}));
    app.pushCmd(cp.createGraphicsPipeline.request("pipeline", "layout", "fullscreen_tri.spv", "rings", "rpass", vk::Rect2D{{0, 0}, {1024, 1024}}, false));
    app.pushCmd(cp.createRenderLayer.request("canvas", "rpass", "pipeline"));
    app.pushCmd(cp.recordRenderCommand.request(0, std::vector<uint32_t>{0}));
    app.pushCmd(cp.render.request(0, 0));
    app.pushCmd(cp.saveAttachmentToPng.request("canvas", path, 0, 0, 1024, 1024, true));

    app.run(false);
    std::cout << "Done." << '\n';
    return 0;
}
