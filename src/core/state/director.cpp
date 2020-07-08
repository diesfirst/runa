#include <state/director.hpp>
#include <event/event.hpp>
#include <util/stringutil.hpp>
#include <thread>

namespace sword
{

namespace state
{

QuickSetup::QuickSetup(StateArgs sa, Callbacks cb) :
    BriefState{sa, cb}, sr{sa.rg}, cp{sa.cp}
{}

void QuickSetup::onEnterExt()
{
    vk::DescriptorSetLayoutBinding binding;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    binding.setBinding(0);
    binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
    binding.setDescriptorCount(1);
    bindings.push_back(binding);
    pushCmd(cp.openWindow.request());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pushCmd(cp.loadFragShader.request(sr.loadFragShaders->reportCallback(), "simpleFrag.spv"));
    pushCmd(cp.loadVertShader.request(sr.loadVertShaders->reportCallback(), "triangle.spv"));
    pushCmd(cp.prepareRenderFrames.request(sr.prepareRenderFrames->reportCallback()));
    pushCmd(cp.createDescriptorSetLayout.request(sr.createDescriptorSetLayout->reportCallback(), "foo", bindings));
    pushCmd(cp.createFrameDescriptorSets.request(sr.createFrameDescriptorSets->reportCallback(), std::vector<std::string>({"foo"})));
    pushCmd(cp.createSwapchainRenderpass.request(sr.createRenderPass->reportCallback(), "swap"));
    pushCmd(cp.createPipelineLayout.request(sr.createPipelineLayout->reportCallback(), "layout", std::vector<std::string> ({"foo"})));
    pushCmd(cp.createGraphicsPipeline.request(sr.createGraphicsPipeline->reportCallback(), "pipe", "layout", "triangle.spv", "simpleFrag.spv", "swap", vk::Rect2D({0, 0}, {500, 500}), false));
    pushCmd(cp.createRenderLayer.request(sr.createRenderLayer->reportCallback(), "swap", "swap", "pipe"));
    pushCmd(cp.recordRenderCommand.request(0, std::vector<uint32_t>({0})));
    popSelf();
}

Director::Director(StateArgs sa, const StateStack& ss, render::Window& window) :
    BranchState{sa, Callbacks(), {
        {"render_manager", opcast(Op::pushRenderManager)},
        {"print_state_hierarchy", opcast(Op::printHierarchy)},
        {"quick_setup", opcast(Op::quickSetup)},
        {"quick_setup3", opcast(Op::quickSetup3)},
        {"painter", opcast(Op::painter)},
        {"viewer", opcast(Op::viewer)}
    }}, 
    stateStack{ss},
    renderManager{sa, 
        {[this](){activate(opcast(Op::pushRenderManager));}, {}, {}}},
    painter{sa,
        {[this](){activate(opcast(Op::painter));}, {}, {}}},
    viewer{sa, 
        {[this](){activate(opcast(Op::viewer));}, {}}},
    quickState{sa, {}},
    cp{sa.cp},
    sr{sa.rg}
{ 
    activate(opcast(Op::pushRenderManager));
    activate(opcast(Op::printHierarchy));
    activate(opcast(Op::quickSetup));
    activate(opcast(Op::quickSetup3));
    activate(opcast(Op::painter));
    activate(opcast(Op::viewer));
}

void Director::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        Optional option = extractCommand(event);
        if (!option) return;
        switch(opcast<Op>(*option))
        {
            case Op::printHierarchy: printStateHierarchy(); break;
            case Op::pushRenderManager: pushRenderManager(); break;
            case Op::painter: pushState(&painter); deactivate(opcast(Op::painter)); break;
            case Op::quickSetup: quickSetup(); break;
            case Op::quickSetup3: quickSetup3(); break;
            case Op::viewer: pushState(&viewer); deactivate(opcast(Op::viewer)); break;
        }
        event->setHandled();
    }
    if (event->getCategory() == event::Category::Abort)
    {
        popTop();
    }
}

void Director::printStateHierarchy()
{
    std::cout << util::makeHeader("State Stack") << std::endl;
    stateStack.print();
}

void Director::pushRenderManager()
{
    pushState(&renderManager);
    deactivate(opcast(Op::pushRenderManager));
}

void Director::popTop()
{
    if (stateStack.size() > 1)
        editStack.popState();
}

void Director::quickSetup()
{
    pushState(&quickState);
    deactivate(opcast(Op::quickSetup));
}

void Director::quickSetup3()
{
    vk::DescriptorSetLayoutBinding binding;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    binding.setBinding(0);
    binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
    binding.setDescriptorCount(1);
    bindings.push_back(binding);
    pushCmd(cp.openWindow.request());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pushCmd(cp.loadVertShader.request(sr.loadVertShaders->reportCallback(), "fullscreen_tri.spv"));
    pushCmd(cp.prepareRenderFrames.request(sr.prepareRenderFrames->reportCallback()));
    pushCmd(cp.createDescriptorSetLayout.request(sr.createDescriptorSetLayout->reportCallback(), "foo", bindings));
    pushCmd(cp.createFrameDescriptorSets.request(sr.createFrameDescriptorSets->reportCallback(), std::vector<std::string>({"foo"})));
    pushCmd(cp.createSwapchainRenderpass.request(sr.createRenderPass->reportCallback(), "swap"));
    pushCmd(cp.createPipelineLayout.request(sr.createPipelineLayout->reportCallback(), "layout", std::vector<std::string> ({"foo"})));
    deactivate(opcast(Op::quickSetup));
    deactivate(opcast(Op::quickSetup3));
    updateVocab();
}

}; // namespace state

}; // namespace sword
