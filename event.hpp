#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <array>
#include "window.hpp"

enum class EventState : size_t
{
	paint, sculpt, viewer, numStates
};

struct UserInput
{
	int16_t mouseX{0};
	int16_t mouseY{0};
	bool mButtonDown{false};
    int16_t blur{0};
    float r{1.};
    float g{1.};
    float b{1.};
};

class EventHandler
{
public:
	EventHandler(const XWindow&);
	~EventHandler();
	
	void setState(EventState);

	UserInput fetchUserInput(bool block);
	UserInput handleEvent(xcb_generic_event_t* event);
	void handlePainterEvent(xcb_generic_event_t* event);
	void handleSculptEvent(xcb_generic_event_t* event);
	void handleViewerEvent(xcb_generic_event_t* event);

	EventState state = EventState::paint;

private:
	const XWindow& window;
    int16_t mouseX{0};
    int16_t mouseY{0};
	bool mButtonDown = false;
    int16_t blur{0};
    float r{1.};
    float g{1.};
    float b{1.};
	Display* display = XOpenDisplay(NULL);
	xcb_generic_event_t* curEvent{nullptr};
};
