#ifndef WINDOW_H
#define WINDOW_H

#include <unistd.h>
#include <xcb/xcb.h>
#include <vector>
#include <iostream>
#include <string.h>

constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;

class XWindow
{
public:
	XWindow ();

	void open();

	std::vector<int> size;
	
	xcb_generic_event_t* waitForEvent();

	xcb_connection_t* connection;

	xcb_window_t window;

	int16_t mouseX = 0;
	int16_t mouseY = 0;
	bool mButtonDown = false;

	void printMousePosition();

private:
	xcb_screen_t* screen;
	uint32_t values[2];
	uint32_t mask = 0;
	xcb_generic_event_t* event;
	xcb_atom_t wmProtocols;
	xcb_atom_t wmDeleteWin;
	std::string appName = "aurora";
	std::string appClass = "floating";

	void createWindow(const int width, const int height);

	void setEvents();

	void pollEvents();

	void sendNotifications();
	
	void setName();

	void setClass();
};
#endif /* ifndef WINDOW_H */
