#ifndef STATE_DESCRIPTORSETLAYOUTMANAGER_HPP
#define STATE_DESCRIPTORSETLAYOUTMANAGER_HPP

#include <state/state.hpp>
#include <command/rendercommands.hpp>

namespace sword
{

namespace state
{

class SetStateType : public LeafState
{
public:
    const char* getName() const override { return "SetStateType"; }
    void handleEvent(event::Event*) override;
    virtual ~SetStateType() = default;   
    SetStateType(StateArgs, Callbacks, vk::DescriptorSetLayoutBinding* const &);
    vk::DescriptorSetLayoutBinding* const & binding;
private:
    void onEnterExt() override;
    SmallMap<std::string, vk::DescriptorType> options;
};

class SetDescriptorCount : public LeafState
{
public:
    const char* getName() const override { return "SetDescriptorCount"; }
    void handleEvent(event::Event*) override;
    virtual ~SetDescriptorCount() = default;   
    SetDescriptorCount(StateArgs, Callbacks, vk::DescriptorSetLayoutBinding* const &);
    vk::DescriptorSetLayoutBinding* const & binding;
private:
    void onEnterExt() override;
};

class SetShaderStageEntry : public LeafState
{
public:
    const char* getName() const override { return "SetShaderStageEntry"; }
    void handleEvent(event::Event*) override;
    virtual ~SetShaderStageEntry() = default;   
    SetShaderStageEntry(StateArgs, Callbacks, vk::DescriptorSetLayoutBinding* const &);
    vk::DescriptorSetLayoutBinding* const & binding;
private:
    void onEnterExt() override;
    SmallMap<std::string, vk::ShaderStageFlagBits> options;
};

class CreateDescriptorSetLayout : public LeafState
{
public:
    const char* getName() const override { return "CreateDescriptorSetLayout"; }
    void handleEvent(event::Event*) override;
    virtual ~CreateDescriptorSetLayout() = default;   
    CreateDescriptorSetLayout(StateArgs, Callbacks, 
            std::vector<vk::DescriptorSetLayoutBinding>&);
private:
    std::vector<vk::DescriptorSetLayoutBinding>& bindings;
    CommandPool<command::CreateDescriptorSetLayout> cdslPool;
    void onEnterExt() override;
};

//TODO handle this with a leaf state
class DescriptorSetLayoutManager final : public BranchState
{
public:
    enum class Op : Option {createBinding, createDescriptorSetLayout, printReports};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "DescriptorSetLayoutManager"; }
    void handleEvent(event::Event*) override;
    virtual ~DescriptorSetLayoutManager() = default;
    DescriptorSetLayoutManager(StateArgs, Callbacks);
    bool hasCreatedLayout();
private:
    SetStateType setStateType;
    SetDescriptorCount setDescriptorCount;
    SetShaderStageEntry setShaderStageEntry;
    CreateDescriptorSetLayout createDescriptorSetLayout;

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    vk::DescriptorSetLayoutBinding* curBinding;

    Reports<DescriptorSetLayoutReport> reports;

    void createBinding();
    void pushCreateDescSetLayout();
    void printReports();
};


}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_DESCRIPTORSETLAYOUTMANAGER_HPP */
