#include "painter.hpp"
#include "event/event.hpp"
#include "rendermanager.hpp"

namespace sword
{

namespace state
{

Painter::Painter(StateArgs sa, Callbacks cb) :
    BranchState{sa, cb, {
        {"init_basic", opcast(Op::init_basic)}
    }},
    cp{sa.cp},
    sr{sa.rg}
{
    activate(opcast(Op::init_basic));
}

void Painter::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        Optional option = extractCommand(event);
        if (!option) return;
        switch(opcast<Op>(*option))
        {
            case Op::init_basic: initBasic(); break;
        }
    }
}

void Painter::initBasic()
{
    vk::DescriptorSetLayoutBinding binding;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    binding.setBinding(0);
    binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
    binding.setDescriptorCount(1);
    bindings.push_back(binding);
    pushCmd(cp.loadVertShader.request(sr.loadVertShaders->reportCallback(), "fullscreen_tri.spv"));
    pushCmd(cp.compileShader.request(sr.compileShader->reportCallback(), "fragment/brush/spot_xform.frag", "spot"));
    pushCmd(cp.prepareRenderFrames.request(sr.prepareRenderFrames->reportCallback()));
    pushCmd(cp.createDescriptorSetLayout.request(sr.createDescriptorSetLayout->reportCallback(), "foo", bindings));
    pushCmd(cp.createFrameDescriptorSets.request(sr.createFrameDescriptorSets->reportCallback(), std::vector<std::string>({"foo"})));
    pushCmd(cp.createSwapchainRenderpass.request(sr.createRenderPass->reportCallback(), "swap"));
    pushCmd(cp.createPipelineLayout.request(sr.createPipelineLayout->reportCallback(), "layout", std::vector<std::string> ({"foo"})));
    pushCmd(cp.createGraphicsPipeline.request(sr.createGraphicsPipeline->reportCallback(), "brush", "layout", "fullscreen_tri.spv", "spot", "swap", vk::Rect2D({0, 0}, {800, 800}), false));
    pushCmd(cp.createRenderLayer.request(sr.createRenderLayer->reportCallback(), "swap", "swap", "brush"));
//    pushCmd(cp.recordRenderCommand.request(sr.recordRenderCommand->reportCallback(), 0, std::vector<uint32_t>({0})));
    deactivate(opcast(Op::init_basic));
    updateVocab();
}

}; // namespace state

}; // namespace sword
