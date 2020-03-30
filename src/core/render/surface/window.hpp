#ifndef RENDER_SURFACE_WINDOW_HPP_
#define RENDER_SURFACE_WINDOW_HPP_

#include <xcb/xcb.h>
#include <vector>
#include <string>

namespace sword
{

namespace render
{

class Window
{
public:
	Window (uint16_t width, uint16_t height);

    ~Window();

	void open();

    uint16_t getWidth() {return width;}

    uint16_t getHeight() {return height;}

    void close();

    bool isOpen() {return opened;}

	std::vector<int> size;
	
	xcb_generic_event_t* pollEvents() const;

	xcb_generic_event_t* waitForEvent() const;

	xcb_connection_t* connection;

	xcb_window_t window;

private:
	xcb_screen_t* screen;
	uint32_t values[2];
	uint32_t mask = 0;
    uint16_t width, height;
	xcb_generic_event_t* event;
	xcb_atom_t wmProtocols;
	xcb_atom_t wmDeleteWin;
	std::string appName = "aurora";
    std::string appClass = "floating";
    bool created{false};
    bool opened{false};

	void createWindow(const int width, const int height);

	void setEvents();

	void setName();

	void setClass();
};

}; // namespace render

}; // namespace sword

;
#endif /* ifndef WINDOW_H */
