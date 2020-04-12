#include <application.hpp>
#include <state/director.hpp>
#include <event/event.hpp>
#include <state/state.hpp>

namespace sword
{

Application::Application(uint16_t w, uint16_t h, const std::string logfile, int eventPops) :
    window{w, h},
    dispatcher{window},
    renderer{context},
    offscreenDim{{w, h}},
    swapDim{{w, h}},
    readlog{logfile},
    dirState{stateEdits, cmdStack, stateStack, window}
{
    stateStack.push(&dirState);
    stateStack.top()->onEnter();

    if (readlog != "eventlog")
    {
        std::cout << "reading events from " << readlog << std::endl;
        recordevents = true;
        readevents = true;
    }
    if (readevents) readEvents(is, eventPops);
    if (recordevents) std::remove(writelog.data());
}

void Application::popState()
{
    stateStack.top()->onExit();
    stateStack.pop();
    auto top = stateStack.top();
    auto topBranch = dynamic_cast<state::BranchState*>(top);
    if (topBranch)
        topBranch->onResume();
    else
        std::cout << "we had multiple leaf states in tree. should not be possible." << std::endl;
    dispatcher.updateVocab();
}

void Application::pushState(state::State* state)
{
    auto top = dynamic_cast<state::BranchState*>(stateStack.top());
    if (top)
        top->onPush();
    else 
    {
        std::cout << "cannout push onto a leaf state" << std::endl;
        return;
    }
    stateStack.push(std::move(state));
    state->onEnter();
    dispatcher.updateVocab();
}

void Application::createPipelineLayout()
{
    const std::string layoutName = "layout";
    std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
    bindings[0].setBinding(0);
    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[0].setDescriptorCount(1);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    bindings[1].setBinding(1);
    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    bindings[1].setDescriptorCount(1); //arbitrary
    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    auto paintingLayout = renderer.createDescriptorSetLayout("paintLayout", bindings);
    renderer.createFrameDescriptorSets({paintingLayout}); //order matters for access
    auto paintPLayout = renderer.createPipelineLayout(layoutName, {paintingLayout});
    std::cout << "Created Pipeline Layout: " << layoutName << std::endl;
}

void Application::loadDefaultShaders()
{
    auto& offScreenLoadPass = renderer.createRenderPass("offscreen_load");
    renderer.prepareAsOffscreenPass(offScreenLoadPass, vk::AttachmentLoadOp::eLoad); //can make this a static function of RenderPass
    auto& swapchainPass = renderer.createRenderPass("swapchain");
    renderer.prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass
    auto& offScreenClearPass = renderer.createRenderPass("offscreen_clear");
    renderer.prepareAsOffscreenPass(offScreenClearPass, vk::AttachmentLoadOp::eClear);
}

void Application::initUBO()
{
    renderer.initFrameUBOs(sizeof(render::FragmentInput), 0); //should be a vector
    renderer.bindUboData(static_cast<void*>(&fragInput), sizeof(render::FragmentInput), 0);
}

void Application::recordEvent(event::Event* event, std::ofstream& os)
{
    if (event->getCategory() == event::Category::CommandLine)
    {

        os.open(writelog, std::ios::out | std::ios::binary | std::ios::app);
        auto ce = static_cast<event::CommandLine*>(event);
        ce->serialize(os);
        os.close();
    }
    if (event->getCategory() == event::Category::Abort)
    {

        os.open(writelog, std::ios::out | std::ios::binary | std::ios::app);
        auto ce = static_cast<event::Abort*>(event);
        ce->serialize(os);
        os.close();
    }
}

void Application::readEvents(std::ifstream& is, int eventPops)
{
    is.open(readlog, std::ios::binary);
    dispatcher.readEvents(is, eventPops);
    is.close();
}

void Application::run()
{
    dispatcher.pollEvents();

    while (1)
    {
        size_t eventCount = dispatcher.eventQueue.size();
        int i = 0;
        while (i < dispatcher.eventQueue.size())
        {
            std::cout << "got events" << std::endl;
            auto& event = dispatcher.eventQueue.items[i];
            if (event)
            {
                if (recordevents) recordEvent(event.get(), os);
                for (auto state : stateStack) 
                    if (!event->isHandled())
                        state->handleEvent(event.get());
            }
            for (auto state : stateEdits) 
            {
                if (state)
                    pushState(state);
                else
                    popState();
            }
            stateEdits.clear();
            i++;
        }
        dispatcher.eventQueue.items.clear();
        for (auto& cmd : cmdStack.items)
        {
            if (cmd)
            {
                cmd->execute(this);
                std::cout << "Command run: " << cmd->getName() << std::endl;
            }
            else
                std::cout << "Recieved null cmd" << std::endl;
        }
        cmdStack.items.clear();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


}; // namespace sword