#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <array>
#include "window.hpp"

enum class MouseButton : uint8_t
{
    Left = 1,
    Middle = 2,
    Right = 3,
};

enum class EventType : uint8_t
{
    Press = 1,
    Release = 2,
    Motion = 3,
    Keypress = 4
};

struct UserInput
{
	int16_t mouseX{0};
	int16_t mouseY{0};
	bool lmButtonDown{false};
    bool mmButtonDown{false};
    bool rmButtonDown{false};
    int16_t blur{0};
    float r{1.};
    float g{1.};
    float b{1.};
    float a{1.};
    float brushSize{1.};
    uint32_t cmdId;
    MouseButton mouseButton;
    EventType eventType;
};

class EventHandler
{
public:
	EventHandler(const XWindow&);
	~EventHandler();

	UserInput& fetchUserInput(bool block);
	UserInput& handleEvent(xcb_generic_event_t* event);
	void handlePainterEvent(xcb_generic_event_t* event);
	void handleSculptEvent(xcb_generic_event_t* event);
	void handleViewerEvent(xcb_generic_event_t* event);

    UserInput state;

private:
	const XWindow& window;
	Display* display = XOpenDisplay(NULL);
	xcb_generic_event_t* curEvent{nullptr};
};
