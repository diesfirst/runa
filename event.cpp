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
#include "sculpter.hpp"
#include "renderer.hpp"
#include "pipe.hpp"
#include "camera.hpp"

typedef void (EventHandler::*pEventFunc)(xcb_generic_event_t* event);

float normCoords(int16_t windowCoord, int16_t extent)
{
	float x = (float)windowCoord / extent; // 0<x<1
	x = x * 2 - 1;
	return x;
}

std::array<pEventFunc, static_cast<size_t>(EventState::numStates)> eventFunctions = 
{
	&EventHandler::handlePainterEvent,
	&EventHandler::handleSculptEvent,
	&EventHandler::handleViewerEvent,
};

EventHandler::EventHandler(
		Commander& commander,
		MemoryManager& mm,
		Painter& painter,
		Swapchain& swapchain,
		Sculpter& sculpter,
		Renderer& renderer,
		Pipe& pipe,
 		Camera& camera)	:
	commander(commander),
	mm(mm),
	painter(painter),
	swapchain(swapchain),
	sculpter(sculpter),
	renderer(renderer),
	pipe(pipe),
	camera(camera)
{
}

EventHandler::~EventHandler()
{
}

void EventHandler::setState(EventState eventState)
{
	state = eventState;
}

void EventHandler::handleEvent(xcb_generic_event_t* event)
{
	if ((event->response_type & ~0x80) == XCB_KEY_PRESS)
	{
		xcb_key_press_event_t* keyPress =
			(xcb_key_press_event_t*)event;
		xcb_keycode_t keycode = keyPress->detail;
		KeySym keysym = XkbKeycodeToKeysym(display, keycode, 0, 0);
		char* key = XKeysymToString(keysym);
		if (strcmp(key, "s") == 0)
		{
			int input;
			std::cout << "Select mode." << std::endl;
			std::cout << "Paint: 0" << std::endl;
			std::cout << "Sculpt: 1" << std::endl;
			std::cout << "Viewer: 2" << std::endl;
			std::cin >> input;
			setState(static_cast<EventState>(input)); //must do checking here);
			std::cout << "CHanged state to: " << static_cast<int>(state) << std::endl;
		}		
	}
	else
	{
		(this->*eventFunctions[static_cast<size_t>(state)])(event);
	}
}

void EventHandler::handleSculptEvent(xcb_generic_event_t* event)
{
	switch (event->response_type & ~0x80)
	{
		case XCB_BUTTON_PRESS:
		{
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			float nX = normCoords(motion->event_x, swapchain.extent.width);
			float nY = normCoords(motion->event_y, swapchain.extent.height);
			sculpter.addPointToGeo(nX, nY);
			sculpter.getCurGeo().printPoints();
			
			commander.recordDrawVert(
					renderer.renderPass, 
					pipe,
					renderer.framebuffers, 
					sculpter.block->buffer, 
					swapchain.extent.width, 
					swapchain.extent.height, 
					sculpter.getCurGeo().points.size());

			commander.renderFrame(swapchain);
			swapchain.context.queue.waitIdle();

			break;
		}
	}
	free(event);
}

void EventHandler::handleViewerEvent(xcb_generic_event_t* event)
{
	switch (event->response_type & ~0x80)
	{
		case XCB_MOTION_NOTIFY:
		{
			if (!mButtonDown) 
				break;
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			//update ubo
			commander.renderFrame(swapchain);
			break;
		}
		case XCB_BUTTON_PRESS:
		{
			mButtonDown = true;
			std::cout << "Hi bub." << std::endl;
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			//update ubo
			commander.renderFrame(swapchain);
			break;
		}
		case XCB_BUTTON_RELEASE:
		{
			mButtonDown = false;
			break;
		}
	}
	free(event);
}

void EventHandler::handlePainterEvent(xcb_generic_event_t* event)
{	
	switch (event->response_type & ~0x80)
	{
		case XCB_MOTION_NOTIFY: 
		{
			if (!mButtonDown) 
				break;
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			painter.paint(motion->event_x, motion->event_y);
			commander.renderFrame(swapchain);
			break;
		}
		case XCB_BUTTON_PRESS:
		{
			mButtonDown = true;
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
			painter.paint(motion->event_x, motion->event_y);
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
				io::saveSwapImage(mm, commander, swapchain);
				break;
			}
			if (strcmp(key, "b") == 0) 
			{
				float radius;
				std::cout << "Enter a new size for brush." << std::endl;
				std::cin >> radius;
				painter.setBrushSize(radius);
				break;
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
				break;
			}
			if (strcmp(key, "n") == 0)
			{
				painter.addNewLayer();
				std::cout << "New layer added" << std::endl;
				std::cout << "Current layer index: " 
					<< painter.getStackSize() - 1 << std::endl;
				break;
			}
			if (strcmp(key, "l") == 0)
			{
				int index;
				std::cout << "Enter a layer index (max is " 
					<< painter.getStackSize() - 1 << ")." << std::endl;
				std::cin >> index;
				if (index > (painter.getStackSize() - 1) || index < 0)
				{
					std::cout << "Index is out of range." << std::endl;
					return;
				}
				painter.switchToLayer(index);
				break;
			}
			if (strcmp(key, "w") == 0)
			{
				painter.writeCurrentLayerToBuffer();
				std::cout << "wrote current layer to buffer" << std::endl;
				commander.renderFrame(swapchain);
				break;
			}
			if (strcmp(key, "f") == 0)
			{
				painter.writeForegroundToBuffer();
				std::cout << "wrote fg to buffer " << std::endl;
				commander.renderFrame(swapchain);
			}
			if (strcmp(key, "g") == 0)
			{
				painter.writeBackgroundToBuffer();
				std::cout << "wrote bg to buffer " << std::endl;
				commander.renderFrame(swapchain);
			}
			if (strcmp(key, "a") == 0)
			{
				float alpha;
				std::cout << "Choose an alpha value between 0 and 1." << std::endl;
				std::cin >> alpha;
				if (alpha < 0 || alpha > 1)
				{
					std::cout << "Input not in between 0 and 1." << std::endl;
					return;
				}
				painter.setAlpha(alpha);
				break;
			}
			if (strcmp(key, "e") == 0)
			{
				painter.toggleErase();
				break;
			}
			break;
		}
	}
	free(event);
}
