#include "viewer.hpp"
#include <event/event.hpp>

namespace sword
{

namespace state
{

Viewer::Viewer(StateArgs sa, Callbacks cb) :
    BranchState{sa, cb, {
            {"load_model", opcast(Op::loadModel)},
            {"initialize", opcast(Op::initialize)}
            }}
{
    activate(opcast(Op::loadModel));
    activate(opcast(Op::initialize));
}

void Viewer::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto option = extractCommand(event);
        if (!option) return;
        switch(opcast<Op>(*option))
        {
            case Op::loadModel: std::cout << "did it" << '\n'; break;
            case Op::initialize: initialize(); break;
        }
    }
}

void Viewer::initialize()
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings(1);
    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[0].setBinding(0);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eVertex);
    bindings[0].setDescriptorCount(1);

    pushCmd(compileShader.request("vertex/triangle.vert", "tri"));
    pushCmd(compileShader.request("fragment/blue.frag", "blue"));
    pushCmd(prepareRenderFrames.request());
    pushCmd(createDescriptorSetLayout.request("foo", bindings));
    pushCmd(createFrameDescriptorSets.request(std::vector<std::string>({"foo"})));
    pushCmd(createSwapchainRenderpass.request("swap"));
    pushCmd(createPipelineLayout.request("layout", std::vector<std::string> ({"foo"})));
    pushCmd(createGraphicsPipeline.request("view", "layout", "tri", "blue", "swap", vk::Rect2D({0, 0}, {800, 800}), false));
    pushCmd(createRenderLayer.request("swap", "swap", "view"));
    pushCmd(addFrameUniformBuffer.request(sizeof(Xform), 0));
    pushCmd(bindUboData.request(&xform, sizeof(Xform), 0));
    pushCmd(recordRenderCommand.request(0, std::vector<uint32_t>{0}));
    pushCmd(openWindow.request());

    deactivate(opcast(Op::initialize));
}

}; // namespace state

}; // namespace sword
