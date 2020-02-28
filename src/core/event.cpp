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

    if (input.empty())
    {
        auto event = std::make_unique<Nothing>();
        eventQueue.emplace(std::move(event));
        std::cout << "Nothing" << std::endl;
        return;
    }

    std::stringstream ss{input};
    std::string catcher;

    while (ss >> catcher)
    {
        if (catcher == "q")
        {
            auto event = std::make_unique<Abort>();
            eventQueue.emplace(std::move(event));
            std::cout << "Aborting operation" << std::endl;
            return;
        }
    }

    auto event = std::make_unique<CommandLineEvent>(input);
    eventQueue.emplace(std::move(event));
}

void EventHandler::fetchWindowInput()
{	
    auto* event = window.waitForEvent();
    std::unique_ptr<Event> curEvent;
	switch (static_cast<WindowEventType>(event->response_type))
	{
        case WindowEventType::Motion: 
		{
			xcb_motion_notify_event_t* motion =
				(xcb_motion_notify_event_t*)event;
            curEvent = std::make_unique<MouseMotionEvent>(motion->event_x, motion->event_y);
            break;
		}
        case WindowEventType::MousePress:
		{
            xcb_button_press_event_t* press = 
                (xcb_button_press_event_t*)event;
            curEvent = std::make_unique<MousePressEvent>(static_cast<MouseButton>(press->detail));
            break;
		}
        case WindowEventType::MouseRelease:
		{
            xcb_button_press_event_t* press = 
                (xcb_button_press_event_t*)event;
            curEvent = std::make_unique<MouseReleaseEvent>(static_cast<MouseButton>(press->detail));
            break;
		}
        case WindowEventType::Keypress:
		{
			xcb_key_press_event_t* keyPress =
				(xcb_key_press_event_t*)event;
            //will cause this to be the last iteration of the window loop
            if (static_cast<Key>(keyPress->detail) == Key::Esc)
                keepWindowThread = false;
            curEvent = std::make_unique<KeyPressEvent>(static_cast<Key>(keyPress->detail));
            break;
		}
        case WindowEventType::Keyrelease:
        {
            xcb_button_release_event_t* keyRelease = 
                (xcb_button_release_event_t*)event;
            curEvent = std::make_unique<KeyReleaseEvent>(static_cast<Key>(keyRelease->detail));
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
    eventQueue.emplace(std::move(curEvent));
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
