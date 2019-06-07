#include "window.hpp"

Window::Window()
{
	connection = xcb_connect(NULL,NULL);
	screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	window = xcb_generate_id(connection);
	setEvents();
	createWindow(500, 500);
}

xcb_connection_t* Window::getConnection()
{
	return connection;
}

xcb_window_t Window::getWindow()
{
	return window;
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
	values[0] = screen->white_pixel;
	values[1] = XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_ENTER_WINDOW |
		XCB_EVENT_MASK_LEAVE_WINDOW;
}

void Window::pollEvents()
{
	xcb_generic_event_t* event;
	while ((event = xcb_poll_for_event(connection)))
	{
		//stuff
	}
}

void Window::waitForEvents()
{
	while ((event = xcb_wait_for_event(connection)))
	{
		free(event);
	}
}

void Window::open()
{
	xcb_map_window(connection, window);
	xcb_flush(connection);
	waitForEvents();
}
