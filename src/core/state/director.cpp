#include <state/director.hpp>
#include <event/event.hpp>
#include <util/stringutil.hpp>

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
    binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    binding.setBinding(0);
    binding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
    binding.setDescriptorCount(1);
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    bindings.push_back(binding);
    pushCmd(cp.loadFragShader.request(sr.loadFragShaders->reportCallback(), "death.spv"));
    pushCmd(cp.loadVertShader.request(sr.loadVertShaders->reportCallback(), "fullscreen_tri.spv"));
    pushCmd(cp.prepareRenderFrames.request(sr.prepareRenderFrames->reportCallback()));
    pushCmd(cp.createDescriptorSetLayout.request(sr.createDescriptorSetLayout->reportCallback(), "foo", bindings));
    pushCmd(cp.createFrameDescriptorSets.request(sr.createFrameDescriptorSets->reportCallback(), std::vector<std::string>({"foo"})));
    pushCmd(cp.createSwapchainRenderpass.request(sr.createRenderPass->reportCallback(), "swap"));
    popSelf();
}

Director::Director(StateArgs sa, const StateStack& ss, render::Window& window) :
    BranchState{sa, Callbacks(), {
        {"render_manager", opcast(Op::pushRenderManager)},
        {"print_state_hierarchy", opcast(Op::printHierarchy)},
        {"quick_setup", opcast(Op::quickSetup)}
    }}, 
    stateStack{ss},
    renderManager{sa, 
        {[this](){activate(opcast(Op::pushRenderManager));}, {}, {}}},
    quickState{sa, {}}
{ 
    activate(opcast(Op::pushRenderManager));
    activate(opcast(Op::printHierarchy));
    activate(opcast(Op::quickSetup));
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
            case Op::quickSetup: quickSetup(); break;
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

}; // namespace state

}; // namespace sword
