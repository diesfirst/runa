#include "painter.hpp"
#include "event/event.hpp"
#include "rendermanager.hpp"
#include "vulkan/vulkan.hpp"
#include <util/debug.hpp>
#include <glm/gtx/string_cast.hpp>

namespace sword
{

namespace state
{

namespace painter
{

enum class Input : uint8_t
{
    resizeBrush = static_cast<uint8_t>(event::symbol::Key::Alt),
    translate = static_cast<uint8_t>(event::symbol::MouseButton::Middle),
    paint = static_cast<uint8_t>(event::symbol::MouseButton::Left),
    scale = static_cast<uint8_t>(event::symbol::MouseButton::Right),
    rotate = static_cast<uint8_t>(event::symbol::Key::Shift_L)
};

constexpr Input inputCast(event::symbol::Key key) { return static_cast<Input>(key); }
constexpr Input inputCast(event::symbol::MouseButton button) { return static_cast<Input>(button); }
constexpr event::Window* toWindowEvent(event::Event* event) { return static_cast<event::Window*>(event); }

void updateXform(glm::mat4& xform, const Matrices& mats)
{
    xform = glm::inverse(mats.pivot) * mats.scaleRotate * mats.translate * mats.pivot * mats.toCanvasSpace;
}

Rotate::Rotate(StateArgs sa, Callbacks cb, PainterVars& vars) :
    LeafState{sa, cb}, vars{vars}, scaleRot{vars.matrices.scaleRotate}, xform{vars.fragInput.xform},
    renderPool{sa.cp.render}
{
}

void Rotate::onEnterExt()
{
    initX = vars.fragInput.mouseX;
    initY = vars.fragInput.mouseY;
    scaleRotCache = vars.matrices.scaleRotate;
}

void Rotate::handleEvent(event::Event *event)
{
    if (event->getCategory() == event::Category::Window)
    {
        auto we = toWindowEvent(event);
        if (we->getType() == event::WindowEventType::Motion)
        {
            angle = angleScale * (we->getX() / vars.swapWidthFloat - initX + we->getY() / vars.swapHeightFloat - initY);
            scaleRot = scaleRotCache * vars.matrices.translate * glm::rotate(glm::mat4(1.), angle, {0, 0, 1.}) * glm::inverse(vars.matrices.translate);
            updateXform(xform, vars.matrices);
            auto cmd = renderPool.request(vars.viewCmdId, 1);
            pushCmd(std::move(cmd));
            event->setHandled();
            return;
        }
        if (we->getType() == event::WindowEventType::Keyrelease)
        {
            auto kr = static_cast<event::KeyRelease*>(event);
            if (inputCast(kr->getKey()) == Input::rotate)
            {
                event->setHandled();
                popSelf();
                return;
            }
        }
    }
}

Scale::Scale(StateArgs sa, Callbacks cb, PainterVars& vars) :
    LeafState{sa, cb}, vars{vars}, xform{vars.fragInput.xform}, scaleRot{vars.matrices.scaleRotate},
    renderPool{sa.cp.render}
{
}

void Scale::onEnterExt()
{
    initX = vars.fragInput.mouseX;
    initY = vars.fragInput.mouseY;
    scaleRotCache = vars.matrices.scaleRotate;
}

void Scale::handleEvent(event::Event *event)
{
    if (event->getCategory() == event::Category::Window)
    {
        auto we = toWindowEvent(event);
        if (we->getType() == event::WindowEventType::Motion)
        {
            auto x = we->getX() / vars.swapWidthFloat;
            auto y = we->getY() / vars.swapHeightFloat;
            auto scale = x - initX + y - initY + 1.;
            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.), glm::vec3(scale, scale, 1.));
            scaleRot = scaleRotCache * vars.matrices.translate * scaleMatrix * glm::inverse(vars.matrices.translate);
            updateXform(xform, vars.matrices);
            auto cmd = renderPool.request(vars.viewCmdId, 1);
            pushCmd(std::move(cmd));
            event->setHandled();
            return;
        }
        if (we->getType() == event::WindowEventType::MouseRelease)
        {
            SWD_DEBUG_MSG("popping")
            event->setHandled();
            popSelf();
        }
    }
}

Translate::Translate(StateArgs sa, Callbacks cb, PainterVars& vars) : 
    LeafState{sa, cb}, translate{vars.matrices.translate}, xform{vars.fragInput.xform}, vars{vars},
    renderPool{sa.cp.render}
{}

void Translate::onEnterExt()
{
    initPos.x = vars.fragInput.mouseX;
    initPos.y = vars.fragInput.mouseY;
    tranlatePrevious = vars.matrices.translate;
}

void Translate::handleEvent(event::Event *event)
{
    if (event->getCategory() == event::Category::Window)
    {
        auto we = static_cast<event::Window*>(event);
        if (we->getType() == event::WindowEventType::Motion)
        {
            pos.x = we->getX() / vars.swapWidthFloat;
            pos.y = we->getY() / vars.swapHeightFloat;
            SWD_DEBUG_MSG("pos.x: " << pos.x);
            SWD_DEBUG_MSG("pos.y: " << pos.y);
            pos = pos - initPos;
            pos *= -1;
            translate = glm::translate(glm::mat4(1.), glm::vec3(pos.x, pos.y, 0)) * tranlatePrevious;
            updateXform(xform, vars.matrices);
            SWD_DEBUG_MSG("xform: " << glm::to_string(vars.fragInput.xform));
            SWD_DEBUG_MSG("viewCmd: " << vars.viewCmdId);
            auto cmd = renderPool.request(vars.viewCmdId, 1);
            pushCmd(std::move(cmd));
            event->setHandled();
            return;
        }
        if (we->getType() == event::WindowEventType::MouseRelease)
        {
            SWD_DEBUG_MSG("popping")
            event->setHandled();
            popSelf();
        }
    }
}

ResizeBrush::ResizeBrush(StateArgs sa, Callbacks cb, PainterVars& vars) :
    LeafState{sa, cb}, renderPool{sa.cp.render}, 
    brushSize{vars.fragInput.brushSize}, brushPosX{vars.fragInput.brushX}, brushPosY{vars.fragInput.brushY},
    vars{vars}
{}

void ResizeBrush::onEnterExt()
{
    begin = true;
    initBrushSize = brushSize;
    glm::vec4 startPos = glm::vec4(vars.fragInput.mouseX, vars.fragInput.mouseY, 1, 1);
    startingDist = glm::length(startPos);
    startPos = vars.fragInput.xform * startPos;
    brushPosX = startPos.x;
    brushPosY = startPos.y;
    SWD_DEBUG_MSG("Starting dist: " << startingDist);
}

void ResizeBrush::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::Window)
    {
        auto we = static_cast<event::Window*>(event);
        if (we->getType() == event::WindowEventType::Motion)
        {
            glm::vec4 pos = glm::vec4(we->getX() / vars.swapWidthFloat, we->getY() / vars.swapHeightFloat, 1, 1);

            float diff = glm::length(pos) - startingDist;
            diff *= 20;
            brushSize = initBrushSize + diff;

            auto cmd = renderPool.request(vars.brushStaticCmd, 1);
            pushCmd(std::move(cmd));

            event->setHandled();
            return;
        }
        if (we->getType() == event::WindowEventType::Keyrelease)
        {
            auto kr = static_cast<event::KeyRelease*>(event);
            if (inputCast(kr->getKey()) == Input::resizeBrush)
            {
                if (begin)
                    begin = false;
                event->setHandled();
                popSelf();
                return;
            }
        }
        if (we->getType() == event::WindowEventType::LeaveWindow)
        {
            SWD_DEBUG_MSG("Leaving window!");
            event->setHandled();
            popSelf();
            return;
        }
    }
}

Paint::Paint(StateArgs sa, Callbacks cb, PainterVars& vars) :
    LeafState{sa, cb}, pool{sa.cp.render}, brushPosX{vars.fragInput.brushX}, brushPosY{vars.fragInput.brushY},
    vars{vars}
{
}

void Paint::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::Window)
    {
        auto we = static_cast<event::Window*>(event);
        if (we->getType() == event::WindowEventType::Motion && mouseDown)
        {
            pos.x = we->getX() / vars.swapWidthFloat;
            pos.y = we->getY() / vars.swapHeightFloat;
            pos = vars.fragInput.xform * pos;
            brushPosX = pos.x;
            brushPosY = pos.y;
            auto cmd = pool.request(vars.paintCmdId, 1);
            pushCmd(std::move(cmd));
            event->isHandled();
            return;
        }
        if (we->getType() == event::WindowEventType::MousePress)
        {
            if (inputCast(static_cast<event::MousePress*>(we)->getMouseButton()) == Input::paint)
            {
                pos.x = we->getX() / vars.swapWidthFloat;
                pos.y = we->getY() / vars.swapHeightFloat;
                pos = vars.fragInput.xform * pos;
                brushPosX = pos.x;
                brushPosY = pos.y;
                auto cmd = pool.request(vars.paintCmdId, 1);
                pushCmd(std::move(cmd));
                event->isHandled();
                mouseDown = true;
                return;
            }
        }
        if (we->getType() == event::WindowEventType::MouseRelease)
        {
            mouseDown = false;
            event->isHandled();
            return;
        }
    }
    if (event->getCategory() == event::Category::Abort)
    {
        mouseDown = false;
        popSelf();
        event->setHandled();
    }
}

Painter::Painter(StateArgs sa, Callbacks cb) :
    BranchState{sa, cb, {
        {"init_basic", opcast(Op::initBasic)},
        {"paint", opcast(Op::paint)}
    }},
    paint{sa, {}, painterVars},
    resizeBrush{sa, {
        [this](){ displayCanvas(); }
    }, painterVars},
    translate{sa, {}, painterVars},
    scale{sa, {}, painterVars},
    rotate{sa, {}, painterVars},
    cp{sa.cp},
    sr{sa.rg}
{
    activate(opcast(Op::initBasic));
    updateXform(painterVars.fragInput.xform, painterVars.matrices);
}

void Painter::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        Optional option = extractCommand(event);
        if (!option) return;
        switch(opcast<Op>(*option))
        {
            case Op::initBasic: initBasic(); break;
            case Op::paint: pushState(&paint); break;
            case Op::brushResize: pushState(&resizeBrush); break;
        }
        return;
    }
    if (event->getCategory() == event::Category::Window)
    {
        auto we = static_cast<event::Window*>(event);
        if (we->getType() == event::WindowEventType::Keypress)
        {
            auto kp = static_cast<event::KeyPress*>(we);
            if (inputCast(kp->getKey()) == Input::resizeBrush)
            {
                // TODO: need to make sure its active
                SWD_DEBUG_MSG("pushing resize");
                painterVars.fragInput.mouseX = we->getX() / painterVars.swapWidthFloat;
                painterVars.fragInput.mouseY = we->getY() / painterVars.swapHeightFloat;
                pushState(&resizeBrush);
                event->setHandled();
                painterVars.fragInput.sampleIndex = 1;
                return;
            }
            if (inputCast(kp->getKey()) == Input::rotate)
            {
                painterVars.fragInput.mouseX = we->getX() / painterVars.swapWidthFloat;
                painterVars.fragInput.mouseY = we->getY() / painterVars.swapHeightFloat;
                pushState(&rotate);
                event->setHandled();
                return;
            }
        }
        if (we->getType() == event::WindowEventType::MousePress)
        {
            auto mp = static_cast<event::MousePress*>(we);
            if (inputCast(mp->getMouseButton()) == Input::translate)
            {
                painterVars.fragInput.mouseX = mp->getX() / painterVars.swapWidthFloat;
                painterVars.fragInput.mouseY = mp->getY() / painterVars.swapHeightFloat;
                SWD_DEBUG_MSG("pushing translate")
                pushState(&translate);
                event->setHandled();
                return;
            }
            if (inputCast(mp->getMouseButton()) == Input::scale)
            {
                painterVars.fragInput.mouseX = mp->getX() / painterVars.swapWidthFloat;
                painterVars.fragInput.mouseY = mp->getY() / painterVars.swapHeightFloat;
                SWD_DEBUG_MSG("pushing translate")
                pushState(&scale);
                event->setHandled();
                return;
            }
        }
    }
}

void Painter::initBasic()
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[0].setBinding(0);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);
    bindings[0].setDescriptorCount(1);
    bindings[1].setBinding(1);
    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    bindings[1].setDescriptorCount(2);
    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    pushCmd(cp.addAttachment.request("paint", C_WIDTH, C_HEIGHT, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled));
    pushCmd(cp.addAttachment.request("paint_clear", C_WIDTH, C_HEIGHT, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled));
    pushCmd(cp.loadVertShader.request(sr.loadVertShaders->reportCallback(), "fullscreen_tri.spv"));
    pushCmd(cp.compileShader.request(sr.compileShader->reportCallback(), "fragment/brush/simple.frag", "spot"));
    pushCmd(cp.compileShader.request(sr.compileShader->reportCallback(), "fragment/simple_comp.frag", "comp"));
    pushCmd(cp.prepareRenderFrames.request(sr.prepareRenderFrames->reportCallback()));
    pushCmd(cp.createDescriptorSetLayout.request(sr.createDescriptorSetLayout->reportCallback(), "foo", bindings));
    pushCmd(cp.createFrameDescriptorSets.request(sr.createFrameDescriptorSets->reportCallback(), std::vector<std::string>({"foo"})));
    pushCmd(cp.createSwapchainRenderpass.request(sr.createRenderPass->reportCallback(), "swap"));
    pushCmd(cp.createOffscreenRenderpass.request(sr.createRenderPass->reportCallback(), "paint", vk::AttachmentLoadOp::eLoad));
    pushCmd(cp.createOffscreenRenderpass.request(sr.createRenderPass->reportCallback(), "paint_clear", vk::AttachmentLoadOp::eClear));
    pushCmd(cp.createPipelineLayout.request(sr.createPipelineLayout->reportCallback(), "layout", std::vector<std::string> ({"foo"})));
    pushCmd(cp.createGraphicsPipeline.request(sr.createGraphicsPipeline->reportCallback(), "brush", "layout", "fullscreen_tri.spv", "spot", "paint", vk::Rect2D({0, 0}, {C_WIDTH, C_HEIGHT}), false));
    pushCmd(cp.createGraphicsPipeline.request(sr.createGraphicsPipeline->reportCallback(), "brush_static", "layout", "fullscreen_tri.spv", "spot", "paint_clear", vk::Rect2D({0, 0}, {C_WIDTH, C_HEIGHT}), false));
    pushCmd(cp.createGraphicsPipeline.request(sr.createGraphicsPipeline->reportCallback(), "comp", "layout", "fullscreen_tri.spv", "comp", "swap", vk::Rect2D({0, 0}, {S_WIDTH, S_HEIGHT}), false));
    pushCmd(cp.createRenderLayer.request(sr.createRenderLayer->reportCallback(), "paint", "paint", "brush"));
    pushCmd(cp.createRenderLayer.request(sr.createRenderLayer->reportCallback(), "paint_clear", "paint_clear", "brush_static"));
    pushCmd(cp.createRenderLayer.request(sr.createRenderLayer->reportCallback(), "swap", "swap", "comp"));
    pushCmd(cp.initFrameUbos.request(sizeof(FragmentInput), 0));
    pushCmd(cp.updateFrameSamplers.request(std::vector<std::string>{"paint", "paint_clear"}, 1));
    pushCmd(cp.bindUboData.request(&painterVars.fragInput, sizeof(FragmentInput)));
    pushCmd(cp.recordRenderCommand.request(sr.recordRenderCommand->reportCallback(), 0, std::vector<uint32_t>{0, 2}));
    pushCmd(cp.recordRenderCommand.request(sr.recordRenderCommand->reportCallback(), 1, std::vector<uint32_t>{1, 2}));
    pushCmd(cp.recordRenderCommand.request(sr.recordRenderCommand->reportCallback(), 2, std::vector<uint32_t>{2}));
    pushCmd(cp.openWindow.request(sr.openWindow->reportCallback()));
    pushCmd(cp.watchFile.request("fragment/brush/simple.frag"));

    pushCmd(cp.render.request(1, 0));
    pushCmd(cp.render.request(1, 0));
    pushCmd(cp.render.request(1, 0));

    pushCmd(cp.render.request(0, 0));
    pushCmd(cp.render.request(0, 0));
    pushCmd(cp.render.request(0, 0));

    painterVars.paintCmdId = 0;
    painterVars.viewCmdId = 2;
    painterVars.brushStaticCmd = 1;
    deactivate(opcast(Op::initBasic));
    activate(opcast(Op::paint));
    activate(opcast(Op::brushResize));
    updateVocab();
}

void Painter::displayCanvas()
{
    painterVars.fragInput.sampleIndex = 0;
    auto cmd = cp.render.request(painterVars.viewCmdId, 1);
    pushCmd(std::move(cmd));
}


}; // namespace paint


}; // namespace state

}; // namespace sword
