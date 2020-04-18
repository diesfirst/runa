#include <state/descriptorsetlayoutmanager.hpp>

namespace sword 
{

namespace state
{

SetStateType::SetStateType(StateArgs sa, vk::DescriptorSetLayoutBinding* const & binding) :
    LeafState{sa},
    options{
        {"uniform_buffer", vk::DescriptorType::eUniformBuffer},
        {"combinded_image_sampler", vk::DescriptorType::eCombinedImageSampler}
    },
    binding{binding}
{}

void SetStateType::onEnterExt()
{
    std::cout << "Enter a descriptor type" << std::endl;
    setVocab(options.getStrings());
}

void SetStateType::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto input = ce->getFirstWord();
        auto option = options.findOption(input);
        if (option)
        {
            assert(binding);
            binding->setDescriptorType(*option);
        }
        event->setHandled();
        popSelf();
    }
}

SetDescriptorCount::SetDescriptorCount(StateArgs sa, vk::DescriptorSetLayoutBinding* const & binding) :
    LeafState{sa},
    binding{binding}
{}

void SetDescriptorCount::onEnterExt()
{
    std::cout << "Enter the number of descriptors" << std::endl;
}

void SetDescriptorCount::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto stream = ce->getStream();
        int i;
        stream >> i;
        assert( i > 0 );
        binding->setDescriptorCount(i);
        event->setHandled();
        popSelf();
    }
}

SetShaderStageEntry::SetShaderStageEntry(StateArgs sa, vk::DescriptorSetLayoutBinding* const & binding) :
    LeafState{sa},
    options{
        {"fragment_shader", vk::ShaderStageFlagBits::eFragment}
    },
    binding{binding}
{}

void SetShaderStageEntry::onEnterExt()
{
    std::cout << "Enter a shader stage" << std::endl;
    setVocab(options.getStrings());
}

void SetShaderStageEntry::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto input = ce->getFirstWord();
        auto option = options.findOption(input);
        if (option)
        {
            std::cout << "Got option" << std::endl;
            binding->setStageFlags(*option);
        }
        event->setHandled();
        popSelf();
    }
}

CreateDescriptorSetLayout::CreateDescriptorSetLayout(StateArgs sa, 
        ReportCallbackFn cb, std::vector<vk::DescriptorSetLayoutBinding>& bindings) :
    LeafState{sa, cb},
    bindings{bindings}
{}

void CreateDescriptorSetLayout::onEnterExt()
{
    std::cout << "Enter a name for the descriptor set layout" << std::endl;
}

void CreateDescriptorSetLayout::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto name = ce->getInput();
        auto cmd = cdslPool.request(name, bindings);
        auto report = new DescriptorSetLayoutReport(name, bindings);
        pushCmd(std::move(cmd));
        reportCallback(report);
        event->setHandled();
        popSelf();
        bindings.clear();
    }
}

DescriptorSetLayoutManager::DescriptorSetLayoutManager(StateArgs sa, ExitCallbackFn cb) :
    BranchState{sa, cb, 
        {
            {"create_binding", opcast(Op::createBinding)},
            {"create_descriptor_set_layout", opcast(Op::createDescriptorSetLayout)},
            {"print_report", opcast(Op::printReports)}
        }},
    setStateType{sa, curBinding},
    setDescriptorCount{sa, curBinding},
    setShaderStageEntry{sa, curBinding},
    createDescriptorSetLayout{sa, 
        std::bind(&BranchState::addReport<DescriptorSetLayoutReport>, this, std::placeholders::_1, &reports),
        bindings
    }
{
    activate(opcast(Op::createBinding));
    activate(opcast(Op::printReports));
}

void DescriptorSetLayoutManager::createBinding()
{
    bindings.push_back(vk::DescriptorSetLayoutBinding());
    curBinding = &bindings.back();
    curBinding->setBinding(bindings.size() - 1);
    pushState(&setShaderStageEntry);
    pushState(&setDescriptorCount);
    pushState(&setStateType);
    activate(opcast(Op::createDescriptorSetLayout));
}

void DescriptorSetLayoutManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto option = extractCommand(event);
        if (!option) return;
        switch (opcast(*option))
        {
            case Op::createBinding: createBinding(); break;
            case Op::createDescriptorSetLayout: pushCreateDescSetLayout(); break;
            case Op::printReports: printReports(); break;
        }
    }
}

void DescriptorSetLayoutManager::pushCreateDescSetLayout()
{
    if (bindings.size() < 1)
    {
        std::cout << "Need to create some bindings first" << std::endl;
        return;
    }
    pushState(&createDescriptorSetLayout);
}

bool DescriptorSetLayoutManager::hasCreatedLayout()
{
    return (reports.size() > 0);
}

void DescriptorSetLayoutManager::printReports()
{
    for (const auto& report : reports) 
    {
        std::invoke(*report);
    }
}

}; // namespace state

}; // namespace 
