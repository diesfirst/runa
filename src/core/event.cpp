#include "event.hpp"
#include <X11/XKBlib.h>
#include <string>
#include <iostream>
#include <bitset>

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

UserInput& EventHandler::fetchUserInput(bool block)
{
    if (block)
    {
        auto event = window.waitForEvent();
        handleEvent(event); //modify the state
        return state;
    }
    else
    {
        auto event = window.pollEvents();
        if(event)
            handleEvent(event);
        return state;
    }
}

UserInput& EventHandler::handleEvent(xcb_generic_event_t* event)
{	
    state.eventType = static_cast<EventType>(event->response_type);
	switch (state.eventType)
	{
        case EventType::Motion: 
		{
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			state.mouseX = motion->event_x;
            state.mouseY = motion->event_y;
            break;
		}
        case EventType::MousePress:
		{
            xcb_button_press_event_t* press = 
                (xcb_button_press_event_t*)event;
            switch (static_cast<MouseButton>(press->detail))
            {
                case MouseButton::Left:
                {
                    state.lmButtonDown = true;
                    break;
                }
                case MouseButton::Middle:
                {
                    state.mmButtonDown = true;
                    break;
                }
                case MouseButton::Right:
                {
                    state.rmButtonDown = true;
                    break;
                }
            }
            break;
		}
        case EventType::MouseRelease:
		{
            xcb_button_press_event_t* press = 
                (xcb_button_press_event_t*)event;
            switch (static_cast<MouseButton>(press->detail))
            {
                case MouseButton::Left:
                {
                    state.lmButtonDown = false;
                    break;
                }
                case MouseButton::Middle:
                {
                    state.mmButtonDown = false;
                    break;
                }
                case MouseButton::Right:
                {
                    state.rmButtonDown = false;
                    break;
                }
            }
            break;
		}
        case EventType::Keypress:
		{
			xcb_key_press_event_t* keyPress =
				(xcb_key_press_event_t*)event;
            std::cout << "Key: " << uint32_t(keyPress->detail) << std::endl;
            state.key = static_cast<Key>(keyPress->detail);
            break;
		}
        case EventType::Keyrelease:
        {
            xcb_button_release_event_t* keyRelease = 
                (xcb_button_release_event_t*)event;
            state.key = static_cast<Key>(keyRelease->detail);
            break;
        }
        case EventType::EnterWindow:
        {
            break;
        }
        case EventType::LeaveWindow:
        {
            break;
        }
	}
	free(event);
	return state;
}
