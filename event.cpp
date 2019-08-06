#include "event.hpp"
#include <X11/XKBlib.h>
#include "commander.hpp"
#include "mem.hpp"
#include "swapchain.hpp"
#include "painter.hpp"
#include <string>
#include <iostream>
#include "io.hpp"
#include "util.hpp"

EventHandler::EventHandler(
		Commander& commander,
		MemoryManager& mm,
		Painter& painter,
		Swapchain& swapchain) :
	commander(commander),
	mm(mm),
	painter(painter),
	swapchain(swapchain)
{
}

EventHandler::~EventHandler()
{
}

void EventHandler::handleEvent(xcb_generic_event_t* event)
{	
	switch (event->response_type & ~0x80)
	{
		case XCB_MOTION_NOTIFY: 
		{
			if (!mButtonDown) 
				break;
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			painter.paintForeground(motion->event_x, motion->event_y);
			commander.renderFrame(swapchain);
			break;
		}
		case XCB_BUTTON_PRESS:
		{
			mButtonDown = true;
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			painter.paintForeground(motion->event_x, motion->event_y);
			commander.renderFrame(swapchain);
			break;
		}
		case XCB_BUTTON_RELEASE:
		{
			mButtonDown = false;
			break;
		}
		case XCB_KEY_PRESS:
		{
			xcb_key_press_event_t* keyPress =
				(xcb_key_press_event_t*)event;
			xcb_keycode_t keycode = keyPress->detail;
			KeySym keysym = XkbKeycodeToKeysym(display, keycode, 0, 0);
			char* key = XKeysymToString(keysym);
			if (key == NULL)
			{
				std::cout << "Keysym not defined" << std::endl;
				break;
			}
			if (strcmp(key, "s") == 0)
			{
				std::cout << "Save key pressed" << std::endl;
				saveSwapImage(mm, commander, swapchain);
				break;
			}
			if (strcmp(key, "b") == 0) 
			{
				float radius;
				std::cout << "Enter a new size for brush." << std::endl;
				std::cin >> radius;
				painter.setBrushSize(radius);
			}
			if (strcmp(key, "c") == 0)
			{
				float r,g,b;
				std::string erMsg = "Input not between 0 and 1.";
				std::cout << "Enter a Red value between 0 and 1."	<< std::endl;
				std::cin >> r;
				if (r > 1 || r < 0)
				{
					std::cout << erMsg << std::endl;
					return;
				}
				std::cout << "Enter a Green value between 0 and 1."	<< std::endl;
				std::cin >> g;
				if (g > 1 || g < 0)
				{
					std::cout << erMsg << std::endl;
					return;
				}
				std::cout << "Enter a Blue value between 0 and 1."	<< std::endl;
				std::cin >> b;
				if (b > 1 || b < 0)
				{
					std::cout << erMsg << std::endl;
					return;
				}
				painter.setCurrentColor(r,g,b);
			}
			break;
		}
	}
	free(event);
}

//pieces of our timer
//	std::clock_t start, end;
//	int i = 0;

//		window.printMousePosition();
//		start = std::clock();
//		end = std::clock();
//		std::cout << "Delta = " << (start - end) / (double) CLOCKS_PER_SEC << std::endl;
//		std::cout << "Loop iteration: " << i << std::endl;
//		i++;
