#include "window.hpp"

Window::Window()
{
	connection = xcb_connect(NULL,NULL);
	screen = xcb_setup_roots_iterator(
			xcb_get_setup(connection)).data;
	window = xcb_generate_id(connection);
	setEvents();
	createWindow(500, 500);
	setName();
	setClass();
}

void Window::createWindow(const int width, const int height)
{
	//create window
	size.push_back(width);
	size.push_back(height);
	std::cout << "set size"  << std::endl;
	xcb_create_window(
			connection,
			XCB_COPY_FROM_PARENT,
			window,
			screen->root,
			0,0, //x, y
			width, height, //width, height
			5, //border width
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual,
			mask, values);
}

void Window::setEvents()
{
	mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	values[0] = screen->black_pixel;
	values[1] = XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_ENTER_WINDOW |
		XCB_EVENT_MASK_KEY_PRESS |
		XCB_EVENT_MASK_LEAVE_WINDOW;
}

void Window::setName()
{	
	xcb_change_property(
		connection,
		XCB_PROP_MODE_REPLACE,
		window,
		XCB_ATOM_WM_NAME,
		XCB_ATOM_STRING,
		8,
		appName.length(),
		appName.c_str());
}

void Window::setClass()
{
	xcb_change_property(
		connection,
		XCB_PROP_MODE_REPLACE,
		window,
		XCB_ATOM_WM_CLASS,
		XCB_ATOM_STRING,
		8,
		appClass.length(),
		appClass.c_str());
}

void Window::pollEvents()
{
	xcb_generic_event_t* event;
	while ((event = xcb_poll_for_event(connection)))
	{
		//stuff
	}
}

void Window::waitForEvent()
{
	event = xcb_wait_for_event(connection);
	switch (event->response_type & ~0x80)
	{
		case XCB_MOTION_NOTIFY: 
		{
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			mouseX = motion->event_x;
			mouseY = motion->event_y;
			break;
		}
		case XCB_KEY_PRESS:
		{
			std::cout << "Key pressed" << std::endl;
		}
	}
	free(event);
}

void Window::printMousePosition()
{
	std::cout << "Mouse X: " << mouseX << std::endl;
	std::cout << "Mouse Y: " << mouseY << std::endl;
}

void Window::open()
{
	sendNotifications();
	xcb_map_window(connection, window);
	xcb_flush(connection);
}

void Window::sendNotifications() 
{
	xcb_intern_atom_cookie_t wmDeleteCookie = xcb_intern_atom(
			connection, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
	xcb_intern_atom_cookie_t wmProtocolsCookie =
	    xcb_intern_atom(connection, 0, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
	xcb_intern_atom_reply_t *wmDeleteReply =
	    xcb_intern_atom_reply(connection, wmDeleteCookie, NULL);
	xcb_intern_atom_reply_t *wmProtocolsReply =
	    xcb_intern_atom_reply(connection, wmProtocolsCookie, NULL);
	wmDeleteWin = wmDeleteReply->atom;
	wmProtocols = wmProtocolsReply->atom;

	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window,
			    wmProtocolsReply->atom, 4, 32, 1, &wmDeleteReply->atom);
}

