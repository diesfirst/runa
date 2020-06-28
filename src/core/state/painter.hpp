//imp: painter.cpp
#ifndef _STATE_PAINTER_HPP
#define _STATE_PAINTER_HPP

#include <bits/stdint-uintn.h>
#define GLM_ENABLE_EXPERIMENTAL

#include "command/rendercommands.hpp"
#include "state.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <types/region.hpp>
#include "command/transfer.hpp"
#include "render/resource.hpp"

namespace sword
{

namespace state
{

namespace painter
{

constexpr uint32_t S_WIDTH = 800;
constexpr uint32_t S_HEIGHT = 800;
constexpr uint32_t C_WIDTH = 1600;
constexpr uint32_t C_HEIGHT = 1600;
constexpr float cMapX = float(S_WIDTH) / float(C_WIDTH);
constexpr float cMapY = float(S_HEIGHT) / float(C_HEIGHT);

// seems that bad things happen when sizeof(FragmentInput) is not a multiple of 4
// TODO: we need to make sure we are not writing to this when a frame is not done
// reading from it. need to have either a lock on it or have 3 that we update 
// independently, corresponding to the current frame
struct FragmentInput
{
    float time{0};
    float mouseX{0};
    float mouseY{0};	
    float r{1.};
    float g{1.};
    float b{1.};
    float a{1.};
    float brushSize{1.};
    float brushX; //mouseX in canvas space
    float brushY; //mouseY in canvas space
    int sampleIndex{0};
    float null2;
    glm::mat4 xform{1.};
};

struct PaintSample
{
    float x;
    float y;
};

struct Matrices
{
    glm::mat4 toCanvasSpace = glm::scale(glm::mat4(1.), glm::vec3(cMapX, cMapY, 1.0)); 
    glm::mat4 translate{1.};
    glm::mat4 pivot = glm::translate(glm::mat4(1.), glm::vec3{-0.5 * cMapX, -0.5 * cMapY, 0.0}); 
    glm::mat4 scaleRotate{1.};
};

struct PainterVars
{
    uint32_t swapWidth{S_WIDTH};
    uint32_t swapHeight{S_HEIGHT};
    float swapWidthFloat{S_WIDTH};
    float swapHeightFloat{S_HEIGHT};
    Matrices matrices;
    FragmentInput fragInput; //should be an array
    int paintCmdId;
    int viewCmdId;
    int brushStaticCmd;
};

using CopyAttachmentToImage = Pool<command::CopyAttachmentToImage, command::Command, 2>;
using CopyImageToAttachment = Pool<command::CopyImageToAttachment, command::Command, 2>;

class Rotate : public LeafState
{
public:
    const char* getName() const override { return "Rotate"; }
    void handleEvent(event::Event*) override;
    Rotate(StateArgs, Callbacks, PainterVars&);
private:
    void onEnterExt() override;
    float initX, initY;
    float angleScale{3};
    float angle{0};
    glm::mat4 scaleRotCache;
    glm::mat4& xform;
    glm::mat4& scaleRot;
    const PainterVars& vars;
    CommandPool<command::Render>& renderPool;
};

class Scale : public LeafState
{
public:
    const char* getName() const override { return "Scale"; }
    void handleEvent(event::Event*) override;
    Scale(StateArgs, Callbacks, PainterVars&);
private:
    void onEnterExt() override;
    const PainterVars& vars;
    float initX, initY;
    glm::mat4 scaleRotCache;
    glm::mat4& scaleRot;
    glm::mat4& xform;
    CommandPool<command::Render>& renderPool;
};

class Translate : public LeafState
{
public:
    const char* getName() const override { return "Translate"; }
    void handleEvent(event::Event*) override;
    Translate(StateArgs, Callbacks, PainterVars& vars);
private:
    void onEnterExt() override;
    glm::mat4& translate;
    glm::mat4 tranlatePrevious;
    glm::mat4& xform;
    const PainterVars& vars;
    glm::vec4 pos{0, 0, 0, 0,};
    glm::vec4 initPos{0, 0, 0, 0};

    bool initial{false};
    CommandPool<command::Render>& renderPool;
};

class ResizeBrush : public LeafState
{
public:
    ResizeBrush(StateArgs, Callbacks, PainterVars&);
    const char* getName() const override { return "ResizeBrush"; }
    void handleEvent(event::Event*) override;
private:
    void onEnterExt() override;
    CommandPool<command::Render>& renderPool;
    float& brushPosX;
    float& brushPosY;
    float startingDist{0};
    float initBrushSize{1};
    bool begin{false};
    const PainterVars& vars;

    float& brushSize;
};

class Paint : public LeafState
{
public:
    Paint(StateArgs, Callbacks, PainterVars&, CopyAttachmentToImage&, render::Image&);
    const char* getName() const override { return "Paint"; }
    void handleEvent(event::Event*) override;
private:
    CommandPool<command::Render>& pool;
    glm::vec4 pos{0, 0, 1., 1.};
    float& brushPosX;
    float& brushPosY;
    const PainterVars& vars;
    bool mouseDown{false};
    CopyAttachmentToImage& copyAttachmentToImage;
    render::Image& undoImage;
};

class SaveAttachment : public LeafState
{
public:
    const char* getName() const override { return "SaveAttachment"; }
    void handleEvent(event::Event*) override;
    SaveAttachment(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    CommandPool<command::SaveAttachmentToPng>& pool;
};

class SaveSwap : public LeafState
{
public:
    const char* getName() const override { return "SaveSwap"; }
    void handleEvent(event::Event*) override;
    SaveSwap(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    CommandPool<command::SaveSwapToPng>& pool;
};

class Painter final : public BranchState
{
public:
    const char* getName() const override { return "Painter"; }
    void handleEvent(event::Event*) override;
    Painter(StateArgs, Callbacks);
private:
    enum class Op : Option {initBasic, paint, brushResize, saveAttachmentToPng};

    Paint paint;
    ResizeBrush resizeBrush;
    Translate translate;
    Scale scale;
    Rotate rotate;
    SaveAttachment saveAttachment;
    SaveSwap saveSwap;

    void initBasic();
    void displayCanvas();

    PainterVars painterVars;

    CopyAttachmentToImage copyAttachmentToImage;
    CopyImageToAttachment copyImageToAttachment;
    render::Image undoImage;

    bool paintActive{false};

    CommandPools& cp;
    const state::Register& sr;
};


}; // namespace Painter

}; // namespace state

}; // namespace sword

#endif /* end of include guard: _STATE_PAINTER_HPP */
