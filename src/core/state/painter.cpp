#include "painter.hpp"
#include "event/event.hpp"
#include "render/ubotypes.hpp"
#include "rendermanager.hpp"

namespace sword
{

namespace state
{

Paint::Paint(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, pool{sa.cp.render}
{
}

void Paint::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::Window)
    {
        auto we = static_cast<event::Window*>(event);
        if (we->getType() == event::WindowEventType::Motion)
        {
            assert(fragInput);
            auto input = static_cast<event::MouseMotion*>(we);
            fragInput->mouseX = input->getX() / float(800);
            fragInput->mouseY = input->getY() / float(800);
            auto cmd = pool.request(0, 1);
            pushCmd(std::move(cmd));
            event->isHandled();
            return;
        }
    }
    if (event->getCategory() == event::Category::Abort)
    {
        popSelf();
        event->setHandled();
    }
}

void Paint::setFragInput(render::FragmentInput* input)
{
    fragInput = input;
}

Painter::Painter(StateArgs sa, Callbacks cb) :
    BranchState{sa, cb, {
        {"init_basic", opcast(Op::init_basic)},
        {"paint", opcast(Op::paint)}
    }},
    paint{sa, {}},
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
            case Op::paint: pushState(&paint); break;
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
    pushCmd(cp.initFrameUbos.request(0));
    pushCmd(cp.bindUboData.request(&fragInput, sizeof(render::FragmentInput)));
    pushCmd(cp.recordRenderCommand.request(sr.recordRenderCommand->reportCallback(), 0, std::vector<uint32_t>{0}));

    paint.setFragInput(&fragInput);
    deactivate(opcast(Op::init_basic));
    activate(opcast(Op::paint));
    updateVocab();
}

}; // namespace state

}; // namespace sword
