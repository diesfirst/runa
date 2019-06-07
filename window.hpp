#include <unistd.h>
#include <xcb/xcb.h>
#include <vector>
#include <iostream>

class Window
{
public:
	Window ();

	void open();

	xcb_connection_t* getConnection();

	xcb_window_t getWindow();

	std::vector<int> size;

private:
	xcb_connection_t* connection;
	xcb_screen_t* screen;
	xcb_window_t window;
	uint32_t values[2];
	uint32_t mask = 0;
	xcb_generic_event_t* event;

	void createWindow(const int width, const int height);

	void setEvents();

	void pollEvents();

	void waitForEvents();

};
