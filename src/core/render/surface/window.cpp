#include <render/surface/window.hpp>

namespace sword
{

namespace render
{

Window::Window(uint16_t width, uint16_t height) :
	connection{xcb_connect(NULL,NULL)},
    window{xcb_generate_id(connection)},
    width{width}, height{height}
{
	screen = xcb_setup_roots_iterator(
			xcb_get_setup(connection)).data;
	setEvents();
	createWindow(width, height);
	setName();
	setClass();
}

Window::~Window()
{
    if (created)
    {
        xcb_destroy_window(connection, window);
        xcb_disconnect(connection);
    }
}

void Window::createWindow(const int width, const int height)
{
	//create window
	size.push_back(width);
	size.push_back(height);
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
    created = true;
}

void Window::setEvents()
{
	mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	values[0] = screen->black_pixel;
	values[1] = XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_ENTER_WINDOW |
		XCB_EVENT_MASK_KEY_PRESS |
        XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_LEAVE_WINDOW |
		XCB_EVENT_MASK_BUTTON_PRESS |
		XCB_EVENT_MASK_BUTTON_RELEASE;
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

xcb_generic_event_t* Window::pollEvents() const
{
	return xcb_poll_for_event(connection);
}

xcb_generic_event_t* Window::waitForEvent() const
{
	return xcb_wait_for_event(connection);
}

void Window::open()
{
	xcb_map_window(connection, window);
	xcb_flush(connection);
    opened = true;
}


}; // namespace render

}; // namespace sword

