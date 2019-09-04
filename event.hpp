#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <array>

class Commander;
class MemoryManager;
class Swapchain;
class Painter;
class Sculpter;
class Renderer;
class Pipe;
class Camera;

enum class EventState : size_t
{
	paint, sculpt, viewer, numStates
};

class EventHandler
{
public:
	EventHandler(
			Commander&,
			MemoryManager&,
			Painter&,
			Swapchain&,
			Sculpter&,
			Renderer&,
			Pipe&,
			Camera&);
	~EventHandler();
	
	void setState(EventState);

	void handleEvent(xcb_generic_event_t* event);
	void handlePainterEvent(xcb_generic_event_t* event);
	void handleSculptEvent(xcb_generic_event_t* event);
	void handleViewerEvent(xcb_generic_event_t* event);

	EventState state = EventState::paint;

private:
	Commander& commander;
	MemoryManager& mm;
	Painter& painter;
	Swapchain& swapchain;
	Sculpter& sculpter;
	Renderer& renderer;
	Pipe& pipe;
	Camera& camera;
	bool mButtonDown = false;
	Display* display = XOpenDisplay(NULL);
};
