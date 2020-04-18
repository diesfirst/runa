#include <state/descriptormanager.hpp>

namespace sword
{

namespace state
{


CreateFrameDescriptorSets::CreateFrameDescriptorSets(StateArgs sa, ReportCallbackFn callback) :
    LeafState{sa, callback} 
{}

void CreateFrameDescriptorSets::onEnterExt()
{
    std::cout << "Enter the descriptor set layout names" << std::endl;
}

void CreateFrameDescriptorSets::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto stream = toCommandLine(event)->getStream();
        std::vector<std::string> layoutNames;
        std::string name;
        while (stream >> name)
            layoutNames.push_back(name);
        auto cmd = cfdsPool.request(layoutNames);
        pushCmd(std::move(cmd));
        if (reportCallback)
        {
            auto report = new DescriptorSetReport(layoutNames, DescriptorSetReport::Type::frameOwned);
            std::invoke(reportCallback, report);
        }
        event->setHandled();
        popSelf();
    }
}

InitFrameUbos::InitFrameUbos(StateArgs sa) :
    LeafState{sa}
{}

void InitFrameUbos::onEnterExt()
{
    std::cout << "Enter a binding number" << std::endl;
}

void InitFrameUbos::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto binding = ce->getFirst<int>();
        auto cmd = pool.request(binding);
        pushCmd(std::move(cmd));
        event->setHandled();
        popSelf();
    }
}

UpdateFrameSamplers::UpdateFrameSamplers(StateArgs sa) :
    LeafState{sa}
{}

void UpdateFrameSamplers::onEnterExt()
{
    std::cout << "Enter a binding number" << std::endl;
}

void UpdateFrameSamplers::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto binding = ce->getFirst<int>();
        auto cmd = pool.request(binding);
        pushCmd(std::move(cmd));
        event->setHandled();
        popSelf();
    }
}

DescriptorManager::DescriptorManager(StateArgs sa, ExitCallbackFn cb) :
    BranchState{sa, cb, 
        {
            {"create_frame_descriptor_sets", opcast(Op::createFrameDescriptorSets)},
            {"print_report", opcast(Op::printReports)},
            {"descriptor_set_layout_manager", opcast(Op::descriptorSetLayoutMgr)},
            {"init_frame_ubos", opcast(Op::initFrameUBOs)},
            {"update_frame_samplers", opcast(Op::updateFrameSamplers)}
        }
    },
    createFrameDescriptorSets{sa, 
        std::bind(&BranchState::addReport<DescriptorSetReport>, this, std::placeholders::_1, &reports),
        [this](){ activate(opcast(Op::initFrameUBOs)); }
    },
    descriptorSetLayoutMgr{sa, std::bind(&DescriptorManager::activateDSetLayoutNeeding, this)},
    initFrameUbos{sa},
    updateFrameSamplers{sa}
{
    activate(opcast(Op::printReports));
    activate(opcast(Op::descriptorSetLayoutMgr));
}

void DescriptorManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto cmdEvent = toCommandLine(event);
        auto option = extractCommand(cmdEvent);
        if (!option) return;
        switch (opcast(*option))
        {
            case Op::createFrameDescriptorSets: pushState(&createFrameDescriptorSets); deactivate(opcast(Op::createFrameDescriptorSets)); break;
            case Op::printReports: printReports(); break;
            case Op::descriptorSetLayoutMgr: pushState(&descriptorSetLayoutMgr); deactivate(opcast(Op::descriptorSetLayoutMgr)); break;
            case Op::initFrameUBOs: pushState(&initFrameUbos); break;
            case Op::updateFrameSamplers: pushState(&updateFrameSamplers); break;
        }
    }
}

void DescriptorManager::printReports()
{
    for (const auto& report : reports) 
    {
        std::invoke(*report);
    }
}

void DescriptorManager::activateDSetLayoutNeeding()
{
    if (descriptorSetLayoutMgr.hasCreatedLayout())
        activate(opcast(Op::createFrameDescriptorSets));
}

}; // namespace state

}; // namespace sword
