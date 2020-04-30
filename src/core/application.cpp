#include <application.hpp>
#include <state/director.hpp>
#include <event/event.hpp>
#include <state/state.hpp>
#include <thread>

namespace sword
{

Application::Application(uint16_t w, uint16_t h, const std::string logfile, int eventPops) :
    window{w, h},
    dispatcher{window},
    renderer{context},
    offscreenDim{{w, h}},
    swapDim{{w, h}},
    readlog{logfile},
    dirState{{stateEdits, cmdStack, cmdPools, stateRegister}, stateStack, window}
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

}

void Application::pushState(state::State* state)
{
    stateStack.push(std::move(state));
    if (state->getType() != state::StateType::leaf)
        state->onEnter();
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
    int i;
    int j;
    int k;

    while (1)
    {
        i = 0;
        while (i < dispatcher.eventQueue.size())
        {
            auto& event = dispatcher.eventQueue.items[i];
            if (event)
            {
                std::cout << "Got event: " << event->getName() << '\n';
                if (recordevents) recordEvent(event.get(), os);
                if (event->getCategory() == event::Category::CommandLine)
                {
                    auto ce = state::toCommandLine(event.get());
                    std::cout << ce->getInput() << '\n';
                }

                for (auto state : stateStack) 
                    if (!event->isHandled())
                        state->handleEvent(event.get());
            }
            if (stateEdits.size() > 0)
            {
                j = 0;
                while (j < stateEdits.size())
                {
                    auto state = stateEdits.at(j);
                    if (state)
                        pushState(state);
                    else
                        popState();
                    j++;
                }
                stateEdits.clear();
                // this allows mutiple leaves to be pushed at once, and they get entered in order
                if (stateStack.top()->getType() == state::StateType::leaf)
                    stateStack.top()->onEnter();
            }
            i++;
        }
        dispatcher.eventQueue.items.clear();
        // see below
        if (!cmdStack.empty())
        {
            cmdStack.reverse();
        }
        while (!cmdStack.empty())
        {
            auto cmd = cmdStack.topPtr();
            if (cmd)
            {
                cmd->execute(this);
                std::cout << "Command run: " << cmd->getName() << ", Success status: " << cmd->succeeded() << '\n';
            }
            else
                std::cout << "Recieved null cmd" << std::endl;
            cmdStack.pop();
        }
        cmdStack.items.clear();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// in case you forget. we need to be able to handle cmdPtrs that end up pushing 
// new commands to the stack when they get destructed (returned the pool). so using a stack for the commands
// makes sense. but we also want the execution order to match the order in which the
// commands were pushed. so we need to reverse the stack before we start iterating over it
// this will be interesting to profile. i don't know yet how big the cmdStack is likely to get in practice.
// and whether iteraticing over it effectively twice will make a difference ultimately.


}; // namespace sword
