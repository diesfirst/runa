#include "window.hpp"

Window::Window()
{
	createWindow();
}

void Window::createWindow()
{
	connection = xcb_connect(NULL,NULL);
	screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	window = xcb_generate_id(connection);
	//create window
	xcb_create_window(
			connection,
			XCB_COPY_FROM_PARENT,
			window,
			screen->root,
			0,0, //x, y
			500,500, //width, height
			5, //border width
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual,
			0, NULL);
	//Map window to screen
	xcb_map_window(connection, window);
	xcb_flush(connection);
	pause();
}
