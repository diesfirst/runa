#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>

class Commander;
class MemoryManager;
class Swapchain;
class Painter;

class EventHandler
{
public:
	EventHandler(
			Commander&,
			MemoryManager&,
			Painter&,
			Swapchain&);
	~EventHandler();

	void handleEvent(xcb_generic_event_t* event);

private:
	Commander& commander;
	MemoryManager& mm;
	Painter& painter;
	Swapchain& swapchain;
	bool mButtonDown = false;
	Display* display = XOpenDisplay(NULL);
};
