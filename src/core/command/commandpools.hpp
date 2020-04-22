#ifndef COMMAND_COMMANDPOOLS_HPP
#define COMMAND_COMMANDPOOLS_HPP

#include <command/rendercommands.hpp>
#include <command/commandtypes.hpp>

namespace sword
{

//not including vocab commands
struct CommandPools
{
    CommandPool<command::LoadFragShader> loadFragShader;
    CommandPool<command::LoadVertShader> loadVertShader;
    CommandPool<command::SetSpecFloat> setSpecFloat;
    CommandPool<command::SetSpecInt> setSpecInt;
//    CommandPool<command::AddAttachment> addAttachment; not implememnted yet
    CommandPool<command::OpenWindow> openWindow;
//    CommandPool<command::SetOffscreenDim> setOffscreenDim; not implemented yet
    CommandPool<command::CreateDescriptorSetLayout> createDescriptorSetLayout;
    CommandPool<command::CreatePipelineLayout> createPipelineLayout;
    CommandPool<command::PrepareRenderFrames> prepareRenderFrames;
    CommandPool<command::CreateGraphicsPipeline> createGraphicsPipeline;
    CommandPool<command::CreateSwapchainRenderpass> createSwapchainRenderpass;
    CommandPool<command::CreateOffscreenRenderpass> createOffscreenRenderpass;
    CommandPool<command::CreateRenderpassInstance> createRenderpassInstance;
    CommandPool<command::RecordRenderCommand> recordRenderCommand;
    CommandPool<command::CreateFrameDescriptorSets> createFrameDescriptorSets;
    CommandPool<command::InitFrameUbos> initFrameUbos;
    CommandPool<command::UpdateFrameSamplers> updateFrameSamplers;
    CommandPool<command::Render> render;
};

}; // namespace sword

#endif /* end of include guard: COMMAND_COMMANDPOOLS_HPP */
