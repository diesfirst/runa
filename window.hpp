#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <vector>
#include <iostream>
#include <string.h>

class Window
{
public:
	Window ();

	void open();

	std::vector<int> size;
	
	void waitForEvent();

	xcb_connection_t* connection;

	xcb_window_t window;
	int16_t mouseX = 0;
	int16_t mouseY = 0;

private:
	xcb_screen_t* screen;
	uint32_t values[2];
	uint32_t mask = 0;
	xcb_generic_event_t* event;
	xcb_atom_t wmProtocols;
	xcb_atom_t wmDeleteWin;
	xcb_icccm_wm_hints_t hints;
	std::string appName = "runa";
	std::string appClass = "floating";

	void createWindow(const int width, const int height);

	void setEvents();

	void pollEvents();

	void sendNotifications();
	
	void setName();

	void setClass();
};
