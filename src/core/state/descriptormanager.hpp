#ifndef STATE_DESCRIPTORMANAGER_HPP
#define STATE_DESCRIPTORMANAGER_HPP

#include <state/state.hpp>
#include <state/descriptorsetlayoutmanager.hpp>

namespace sword
{

namespace state
{

using DescriptorReports = Reports<DescriptorSetReport>;

class CreateFrameDescriptorSets : public LeafState
{
public:
    const char* getName() const override { return "CreateFrameDescriptorSets"; }
    void handleEvent(event::Event*) override;
    virtual ~CreateFrameDescriptorSets() = default;   
    CreateFrameDescriptorSets(StateArgs, ReportCallbackFn callback);
private:
    void onEnterExt() override;
    CommandPool<command::CreateFrameDescriptorSets> cfdsPool;
};

class InitFrameUbos : public LeafState
{
public:
    const char* getName() const override { return "InitFrameUbos"; }
    void handleEvent(event::Event*) override;
    virtual ~InitFrameUbos() = default;   
    InitFrameUbos(StateArgs);
private:
    void onEnterExt() override;
    CommandPool<command::InitFrameUbos> pool;
};

class UpdateFrameSamplers : public LeafState
{
public:
    const char* getName() const override { return "UpdateFrameSamplers"; }
    void handleEvent(event::Event*) override;
    virtual ~UpdateFrameSamplers() = default;   
    UpdateFrameSamplers(StateArgs);
private:
    void onEnterExt() override;
    CommandPool<command::UpdateFrameSamplers> pool;
};

class DescriptorManager final : public BranchState
{
public:
    enum class Op : Option {createFrameDescriptorSets, printReports, descriptorSetLayoutMgr, initFrameUBOs, updateFrameSamplers};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "descriptor_manager"; }
    void handleEvent(event::Event*) override;
    virtual ~DescriptorManager() = default;
    DescriptorManager(StateArgs, ExitCallbackFn);
private:
    DescriptorSetLayoutManager descriptorSetLayoutMgr;
    CreateFrameDescriptorSets createFrameDescriptorSets;
    InitFrameUbos initFrameUbos;
    UpdateFrameSamplers updateFrameSamplers;

    DescriptorReports reports;

    void printReports();
    void activateDSetLayoutNeeding();
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_DESCRIPTORMANAGER_HPP */
