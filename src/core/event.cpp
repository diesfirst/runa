#include "event.hpp"
#include <X11/XKBlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <bitset>
#include <functional>
#include <readline/readline.h>
#include <readline/history.h>

typedef void (EventHandler::*pEventFunc)(xcb_generic_event_t* event);

constexpr uint32_t RL_DELAY = 100;
float normCoords(int16_t windowCoord, int16_t extent)
{
	float x = (float)windowCoord / extent; // 0<x<1
	x = x * 2 - 1;
	return x;
}

int abortHelper(int count, int key)
{
    rl_replace_line("q", 0);
    rl_done = 1;
    return 0;
}

EventHandler::EventHandler(const XWindow& window):
	window{window}
{
    rl_attempted_completion_function = completer;
    rl_bind_key(27, abortHelper);
}

EventHandler::~EventHandler()
{
    keepWindowThread = false;
    keepCommandThread = false;
}

char* EventHandler::completion_generator(const char* text, int state)
{
    static std::vector<std::string> matches;
    static size_t match_index = 0;

    if (state == 0)
    {
        matches.clear();
        match_index = 0;
    
        std::string textstr(text);
        for (auto word : vocabulary)
        {
            if (word.size() >= textstr.size() && word.compare(0, textstr.size(), textstr) == 0)
            {
                matches.push_back(word);
            }
        }
    }
    
    if (match_index >= matches.size())
    {
        return nullptr;
    }
    else
    {
        return strdup(matches[match_index++].c_str());
    }
}

char** EventHandler::completer(const char* text, int start, int end)
{
    rl_attempted_completion_over = 1;

    return rl_completion_matches(text, EventHandler::completion_generator);
}

void EventHandler::setVocabulary(std::vector<std::string> vocab)
{
    vocabulary = vocab;
}

void EventHandler::updateVocab()
{
    vocabulary.clear();
    for (const auto& vptr : vocabPtrs) 
        for (const auto& word : *vptr) 
            vocabulary.push_back(word);   
}

void EventHandler::addVocab(const Vocab* vptr)
{
    vocabPtrs.push_back(vptr);
}

void EventHandler::popVocab()
{
    vocabPtrs.pop_back();
}

void EventHandler::fetchCommandLineInput()
{
    CommandLineInput input;

    char* buf = readline(">> ");

    if (buf && *buf) //dont add empty lines to history
        add_history(buf);

    input = std::string(buf);

    free(buf);

    if (input == "quit")
    {
        keepCommandThread = false;   
    }

    std::stringstream ss{input};
    std::string catcher;

    while (ss >> catcher)
    {
        if (catcher == "q")
        {
            auto event = aPool.request();
            eventQueue.push(std::move(event));
            std::cout << "Aborting operation" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(RL_DELAY));
            return;
        }
    }

    auto event = clPool.request(input);
    eventQueue.push(std::move(event));

    std::this_thread::sleep_for(std::chrono::milliseconds(RL_DELAY));
}

void EventHandler::fetchWindowInput()
{	
    auto* event = window.waitForEvent();
    EventPtr curEvent;
	switch (static_cast<WindowEventType>(event->response_type))
	{
        case WindowEventType::Motion: 
		{
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
            curEvent = mmPool.request(motion->event_x, motion->event_y);
            break;
		}
        case WindowEventType::MousePress:
		{
            xcb_button_press_event_t* press = 
                (xcb_button_press_event_t*)event;
            curEvent = mpPool.request(press->event_x, press->event_y, static_cast<MouseButton>(press->detail));
            break;
		}
        case WindowEventType::MouseRelease:
		{
            xcb_button_press_event_t* press = 
                (xcb_button_press_event_t*)event;
            curEvent = mrPool.request(press->event_x, press->event_y, static_cast<MouseButton>(press->detail));
            break;
		}
        case WindowEventType::Keypress:
		{
			xcb_key_press_event_t* keyPress =
				(xcb_key_press_event_t*)event;
            //will cause this to be the last iteration of the window loop
            if (static_cast<Key>(keyPress->detail) == Key::Esc)
                keepWindowThread = false;
            curEvent = kpPool.request(keyPress->event_x, keyPress->event_y, static_cast<Key>(keyPress->detail));
            break;
		}
        case WindowEventType::Keyrelease:
        {
            xcb_button_release_event_t* keyRelease = 
                (xcb_button_release_event_t*)event;
            curEvent = kpPool.request(keyRelease->event_x, keyRelease->event_y, static_cast<Key>(keyRelease->detail));
            break;
        }
        case WindowEventType::EnterWindow:
        {
            std::cout << "Entered window!" << std::endl;
            break;
        }
        case WindowEventType::LeaveWindow:
        {
            std::cout << "Left window!" << std::endl;
            break;
        }
	}
	free(event);
    eventQueue.push(std::move(curEvent));
}

void EventHandler::runCommandLineLoop()
{
    while (keepCommandThread)
    {
        fetchCommandLineInput();
    }
    std::cout << "Commandline thread exitted." << std::endl;
}

void EventHandler::runWindowInputLoop()
{
    while(keepWindowThread)
    {
        fetchWindowInput();
    }
    std::cout << "Window thread exitted." << std::endl;
}

void EventHandler::pollEvents()
{
    std::thread t0(&EventHandler::runCommandLineLoop, this);
    std::thread t1(&EventHandler::runWindowInputLoop, this);
    t0.detach();
    t1.detach();
}

static int clCount = 0;

void EventHandler::readEvents(std::ifstream& is, int eventPops)
{
    bool reading{true};
    while (is.peek() != EOF)
    {
        EventCategory ec = Event::unserializeCategory(is);
        if (ec == EventCategory::CommandLine)
        {
            auto event = clPool.request("foo");
            auto cmdevent = static_cast<CommandLineEvent*>(event.get());
            cmdevent->unserialize(is);
            eventQueue.push(std::move(event));
            clCount++;
            std::cout << "pushed commandline event" << std::endl;
            std::cout << "Cl count: "<< clCount << std::endl;
        }
        if (ec == EventCategory::Abort)
        {
            auto abrt = aPool.request();
            eventQueue.push(std::move(abrt));
            std::cout << "pushed abort event" << std::endl;
        }
    }
    for (int i = 0; i < eventPops; i++) 
    {
        eventQueue.pop();
        std::cout << "popped from queue" << std::endl;
    }
}

