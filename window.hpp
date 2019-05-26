#include <unistd.h>
#include <xcb/xcb.h>

class Window
{
public:
	Window ();

private:
	xcb_connection_t* connection;
	xcb_screen_t* screen;
	xcb_window_t window;

	void createWindow();
};
