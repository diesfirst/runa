#ifndef STATE_DESCRIPTORMANAGER_HPP
#define STATE_DESCRIPTORMANAGER_HPP

//imp: state/descriptormanager.cpp

#include <state/state.hpp>
#include <state/descriptorsetlayoutmanager.hpp>

namespace sword
{

namespace state
{

class CreateFrameDescriptorSets : public LeafState
{
public:
    const char* getName() const override { return "CreateFrameDescriptorSets"; }
    void handleEvent(event::Event*) override;
    virtual ~CreateFrameDescriptorSets() = default;   
    CreateFrameDescriptorSets(StateArgs, Callbacks);
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
    const char* getName() const override { return "descriptor_manager"; }
    void handleEvent(event::Event*) override;
    virtual ~DescriptorManager() = default;
    DescriptorManager(StateArgs, Callbacks, ReportCallbackFn<DescriptorSetLayoutReport>);
private:
    enum class Op : Option {createFrameDescriptorSets, printReports, descriptorSetLayoutMgr, initFrameUBOs, updateFrameSamplers};

    DescriptorSetLayoutManager descriptorSetLayoutMgr;
    CreateFrameDescriptorSets createFrameDescriptorSets;
    InitFrameUbos initFrameUbos;
    UpdateFrameSamplers updateFrameSamplers;

    Reports<DescriptorSetReport> descriptorSetReports; //ownding
    std::vector<const DescriptorSetLayoutReport*> descriptorSetLayoutReports;

    ReportCallbackFn<DescriptorSetLayoutReport> receivedDSLReportCb;

    void printReports();
    void activateDSetLayoutNeeding();
    void receiveDescriptorSetLayoutReport(const DescriptorSetLayoutReport*);
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_DESCRIPTORMANAGER_HPP */
