#include "event.hpp"
#include <X11/XKBlib.h>
#include <string>
#include <iostream>

typedef void (EventHandler::*pEventFunc)(xcb_generic_event_t* event);

float normCoords(int16_t windowCoord, int16_t extent)
{
	float x = (float)windowCoord / extent; // 0<x<1
	x = x * 2 - 1;
	return x;
}

EventHandler::EventHandler(const XWindow& window):
	window{window}
{
}

EventHandler::~EventHandler()
{
}

void EventHandler::setState(EventState eventState)
{
	state = eventState;
}

UserInput EventHandler::fetchUserInput(bool block)
{
	auto event = window.waitForEvent();
	return handleEvent(event);
}

UserInput EventHandler::handleEvent(xcb_generic_event_t* event)
{	
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
		case XCB_BUTTON_PRESS:
		{
			mButtonDown = true;
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			mouseX = motion->event_x;
            mouseY = motion->event_y;
            break;
		}
		case XCB_BUTTON_RELEASE:
		{
			mButtonDown = false;
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			mouseX = motion->event_x;
            mouseY = motion->event_y;
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
                break;
			}
			if (strcmp(key, "b") == 0) 
			{
				std::cout << "Enter a blur radius." << std::endl;
                std::cin >> blur;
                break;
			}
			if (strcmp(key, "c") == 0) 
			{
				std::cout << "Enter a color." << std::endl;
                std::cout << "R: " << std::endl;
                std::cin >> r;
                std::cout << "G: " << std::endl;
                std::cin >> g;
                std::cout << "B: " << std::endl;
                std::cin >> b;
                break;
			}
		}
	}
	free(event);
	return UserInput({mouseX, mouseY, mButtonDown, blur, r, g, b});
}
