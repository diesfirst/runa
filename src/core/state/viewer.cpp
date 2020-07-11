#include "viewer.hpp"
#include <cstring>
#include <event/event.hpp>
#include <geometry/types.hpp>
#include <render/types.hpp>
#include <thread>
#include <chrono>
#include <cmath>

namespace sword
{

namespace state
{

struct Position
{
    float x;
    float y;
    float z;
};

struct Color
{
    float r;
    float g;
    float b;
};

struct Vertex
{
    Position pos;
    Color color;
};

Vertex vertices[] = 
{
    {{-0.5, -0.5, 0}, {1, 0, 0}}, 
    {{ 0.5,  0.5, 0}, {0, 1, 0}}, 
    {{-0.5,  0.5, 0}, {0, 0, 1}},
    {{-0.3, -0.2, 0}, {1, 0, 0}}, 
    {{ 0.7,  0.1, 0}, {0, 1, 0}}, 
    {{-0.4,  0.6, 0}, {0, 0, 1}},
};

Viewer::Viewer(StateArgs sa, Callbacks cb) :
    BranchState{sa, cb, {
            {"load_model", opcast(Op::loadModel)},
            {"initialize", opcast(Op::initialize)},
            }}
{
    activate(opcast(Op::loadModel));
    activate(opcast(Op::initialize));
}


static float distance(float x1, float y1, float x2, float y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

static int findVertInRange(float x, float y)
{
    size_t count = sizeof(vertices) / sizeof(vertices[0]);
    for (int i = 0; i < count; i++) 
    {
        auto pos = vertices[i].pos;
        float dist = distance(x, y, pos.x, pos.y);
        if (dist < .2)
            return i;
    }
    return -1;
}

static float normalize(int16_t x)
{
    return float(x) / 800.0;
}

void Viewer::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto option = extractCommand(event);
        if (!option) return;
        switch(opcast<Op>(*option))
        {
            case Op::loadModel: std::cout << "loadModel called" << '\n'; break;
            case Op::initialize: initialize(); break;
        }
    }
    if (event->getCategory() == event::Category::Window)
    {
        static float initX = 0.0;
        static float initY = 0.0;
        static float initVertX = 0.0;
        static float initVertY = 0.0;
        static bool mouseButtonDown = false;
        static int curVert = -1;

        auto we = toWindowEvent(event);
        if (we->getType() == event::WindowEventType::MousePress)
        {
            auto mp = toMousePress(we);
            if(mp->getMouseButton() == event::symbol::MouseButton::Left)
            {
                mouseButtonDown = true;
                auto x = normalize(mp->getX());
                auto y = normalize(mp->getY());
                SWD_DEBUG_MSG("x: " << x << " y: " << y);
                auto vert = findVertInRange(x, y);
                if (vert == -1) return;
                SWD_DEBUG_MSG("Selecting vert " << vert);
                mouseButtonDown = true;
                initX = x;
                initY = y;
                initVertX = vertices[curVert].pos.x;
                initVertY = vertices[curVert].pos.y;
                curVert = vert;
                return;
            }
        }
//        else if (we->getType() == event::WindowEventType::MouseRelease)
//        {
//            auto mp = toMousePress(we);
//            if(mp->getMouseButton() == event::symbol::MouseButton::Left)
//            {
//                mouseButtonDown = false;
//                return;
//            }
//        }
        else if (we->getType() == event::WindowEventType::Motion && mouseButtonDown)
        {
            float diffX = normalize(we->getX()) - initX;
            float diffY = normalize(we->getY()) - initY;
            vertices[curVert].pos.x = initVertX + diffX;
            vertices[curVert].pos.y = initVertY + diffY;
            memcpy(stagingVertBuffer->pHostMemory, vertices, sizeof(vertices));
            SWD_DEBUG_MSG("Added an x difference of " << diffX);
            return;
        }
    }
}

void Viewer::initialize()
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings(1);
    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[0].setBinding(0);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eVertex);
    bindings[0].setDescriptorCount(1);

    geo::VertexInfo vertInfo{
        sizeof(Vertex), 
        {offsetof(Vertex, pos), offsetof(Vertex, color)}};

    pushCmd(compileShader.request("vertex/basic.vert", "tri"));
    pushCmd(compileShader.request("fragment/vertcolor.frag", "blue"));
    pushCmd(prepareRenderFrames.request());
    pushCmd(createDescriptorSetLayout.request("foo", bindings));
    pushCmd(createFrameDescriptorSets.request(std::vector<std::string>({"foo"})));
    pushCmd(createSwapchainRenderpass.request("swap"));
    pushCmd(createPipelineLayout.request("layout", std::vector<std::string> ({"foo"})));
    pushCmd(createGraphicsPipeline.request(
                "view", "layout", "tri", "blue", "swap", 
                vk::Rect2D({0, 0}, {800, 800}), vertInfo, vk::PolygonMode::eLine));
    pushCmd(addFrameUniformBuffer.request(sizeof(Xform), 0));
    pushCmd(bindUboData.request(&xform, sizeof(Xform), 0));
    pushCmd(requestBufferBlock.request(
                sizeof(vertices), 
                command::RequestBufferBlock::Type::host, 
                &stagingVertBuffer));

    // poor mans synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    memcpy(stagingVertBuffer->pHostMemory, vertices, sizeof(vertices));

    render::DrawParms drawParms{&stagingVertBuffer, sizeof(vertices) / sizeof(vertices[0]), 0};

    pushCmd(createRenderLayer.request("swap", "swap", "view", drawParms));

    pushCmd(recordRenderCommand.request(0, std::vector<uint32_t>{0}));

    pushCmd(openWindow.request());

    pushCmd(pushDraw.request(0));

    deactivate(opcast(Op::initialize));
}


} // namespace state

} // namespace sword
