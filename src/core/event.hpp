#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <array>
#include "window.hpp"
#include <memory>
#include <thread>
#include <queue>
#include <fstream>
#include <sstream>
#include <optional>
#include <functional>

constexpr uint32_t POOL_DEFAULT_SIZE = 3;

template <class T>
class Stack
{
public:
    friend class Application;
    inline void push(T&& item) {items.push_back(std::forward<T>(item));}
    inline void pop() {items.pop_back();}
    inline T top() const {return items.back();}
    inline bool empty() const {return items.empty();}
    inline void print() const {for (const auto& item : items) std::cout << item->getName() << std::endl;}
    inline size_t size() const {return items.size();}
    inline void emplace_back(T&& t) { items.emplace_back(std::move(t));}
protected:
    std::vector<T> items;
};

template <class T>
class ForwardStack : public Stack<T>
{
public:
    inline auto begin() {return this->items.begin();}
    inline auto end() {return this->items.end();}
};

template <class T>
class ReverseStack : public Stack<T>
{
public:
    inline auto begin() {return this->items.rbegin();}
    inline auto end() {return this->items.rend();}
};

template <typename T, typename Base>
class Pool
{
public:
    template <typename P> using Pointer = std::unique_ptr<P, std::function<void(P*)>>;

    Pool(size_t size) : size{size}, pool(size) {}
    Pool() : size{POOL_DEFAULT_SIZE}, pool(size) {}

    template <typename... Args> Pointer<Base> request(Args... args)
    {
        for (int i = 0; i < size; i++) 
            if (pool[i].isAvailable())
            {
                pool[i].set(args...);
                pool[i].activate();
                Pointer<Base> ptr{&pool[i], [](Base* t)
                    {
                        t->reset();
                    }};
                return ptr; //tentatively may need to be std::move? copy should be ellided tho
            }    
        return nullptr;
    }

    void printAll() const 
    {
        for (auto& i : pool) 
            i.print();
    }

private:

    template <typename... Args> void initialize(Args&&... args)
    {
        for (int i = 0; i < size; i++) 
            pool.emplace_back(args...);   
    }

    const size_t size;
    std::vector<T> pool;
};


enum class MouseButton : uint8_t
{
    Left = 1,
    Middle = 2,
    Right = 3,
};

enum class Key : uint8_t
{
    R = 27,
    Shift_L = 50,
    C = 54,
    B = 56,
    Alt = 64,
    Esc = 9,
    S = 39,
    A = 38,
    P = 33
};

enum class WindowEventType : uint8_t
{
    MousePress = 4,
    MouseRelease = 5,
    Motion = 6,
    Keypress = 2,
    Keyrelease = 3,
    LeaveWindow = 8,
    EnterWindow = 7,
};

struct WindowInput
{
	int16_t mouseX{0};
	int16_t mouseY{0};
    MouseButton mouseButton;
    Key key;
    WindowEventType eventType;
};

typedef std::string CommandLineInput;

enum class EventCategory : uint8_t
{
    CommandLine,
    Window,
    Abort,
    Nothing,
};

class Event
{
public:
    virtual EventCategory getCategory() const = 0;
    virtual std::string getName() const = 0;
    template <typename... Args> void set(Args... args) {}
    inline bool isAvailable() const {return !inUse;}
    inline void setHandled() {handled = true;}
    inline bool isHandled() const {return handled;}
    inline static EventCategory unserializeCategory(std::ifstream& is)
    {
        EventCategory ec;
        is.read((char*)&ec, sizeof(EventCategory));
        if (ec == EventCategory::CommandLine)
            return EventCategory::CommandLine;
        else if (ec == EventCategory::Abort)
            return EventCategory::Abort;
        else
        {
            throw std::runtime_error("Could not unserialize category");
        }
    }
    void reset() {inUse = false; handled = false;}
    void activate() {inUse = true;}
protected:
    bool handled{false};
    bool inUse{false};
};

class Abort : public Event
{
public:
   inline EventCategory getCategory() const override {return EventCategory::Abort;} 
   inline std::string getName() const override {return "Abort";}
   void set() {}
   inline void serialize(std::ofstream& os) 
   {
       auto cat = getCategory();
       os.write((char*)&cat, sizeof(EventCategory));
   }
};

class Nothing : public Event
{
public:
   inline EventCategory getCategory() const override {return EventCategory::Nothing;} 
   inline std::string getName() const override {return "Nothing";}
};

class CommandLineEvent : public Event
{
public:
    void set(CommandLineInput input) { this->input = input;}
    inline EventCategory getCategory() const override {return EventCategory::CommandLine;}
    inline std::string getName() const override {return "CommandLine";};
    inline CommandLineInput getInput() const {return input;}
    inline std::string getFirstWord() const 
    {
        std::stringstream ss{input};
        std::string word; ss >> word;
        return word;
    }
    inline std::stringstream getStream() const
    {
        return std::stringstream{input};
    }
    inline void serialize(std::ofstream& os) {
        auto cat = getCategory();
        os.write((char*)&cat, sizeof(EventCategory));
        size_t inputSize = input.size();
        os.write((char*)&inputSize, sizeof(size_t));
        os.write((char*)input.data(), input.size());
        os.write((char*)&handled, sizeof(bool));
    }
    inline void unserialize(std::ifstream& is) {
        size_t inputSize;
        is.read((char*)&inputSize, sizeof(size_t));
        char cstr[inputSize];
        is.read(cstr, inputSize);
        input.assign(cstr, inputSize);
        is.read((char*)&handled, sizeof(bool));
    }
private:
    CommandLineInput input;
};

class WindowEvent : public Event
{
public:
    inline EventCategory getCategory() const override {return EventCategory::Window;}
    virtual WindowEventType getType() const = 0;
    inline int16_t getX() const {return xPos;}
    inline int16_t getY() const {return yPos;}
protected:
    int16_t xPos, yPos;
};

class KeyEvent : public WindowEvent
{
public:
    Key getKey() const {return key;}

protected:
    Key key;
};

class KeyPressEvent final : public KeyEvent
{
public:
    void set(int16_t x, int16_t y, Key key) {xPos = x; yPos = y; this->key = key;}
    inline WindowEventType getType() const override {return WindowEventType::Keypress;}
    inline std::string getName() const override {return "KeyPressEvent";};
};

class KeyReleaseEvent final : public KeyEvent
{
public:
    void set(int16_t x, int16_t y, Key key) {xPos = x; yPos = y; this->key = key;}
    inline WindowEventType getType() const override {return WindowEventType::Keypress;}
    inline std::string getName() const override {return "KeyReleaseEvent";};
};

class MouseButtonEvent : public WindowEvent 
{
public:
    MouseButton getMouseButton() const {return button;}
protected:
    MouseButton button;
};

class MousePressEvent final : public MouseButtonEvent
{
public:
    void set(int16_t x, int16_t y, MouseButton button) {xPos = x; yPos = y; this->button = button;}
    inline WindowEventType getType() const override {return WindowEventType::MousePress;}
    inline std::string getName() const override {return "MousePressEvent";};
};

class MouseReleaseEvent final : public MouseButtonEvent
{
public:
    void set(int16_t x, int16_t y, MouseButton button) {xPos = x; yPos = y; this->button = button;}
    inline WindowEventType getType() const override {return WindowEventType::MouseRelease;}
    inline std::string getName() const override {return "MouseReleaseEvent";};
};

class MouseMotionEvent final : public WindowEvent 
{
public:
    void set(int16_t x, int16_t y) {xPos = x; yPos = y;}
    inline WindowEventType getType() const override {return WindowEventType::Motion;}
    inline std::string getName() const override {return "MouseMotionEvent";};
};


enum class InputMode : uint8_t
{
    CommandLine,
    Window,
};

using Vocab = std::vector<std::string>;

template <typename T>
using EventPool = Pool<T, Event>;
using EventPtr = std::unique_ptr<Event, std::function<void(Event*)>>;

using EventQueue = ForwardStack<EventPtr>;

//this class is not thread safe at all
class EventHandler
{
public:
	EventHandler(const XWindow&);
	~EventHandler();

    void setVocabulary(std::vector<std::string> vocab);
    static void updateVocab();
    void addVocab(const Vocab*);
    void popVocab();

	void fetchWindowInput();
    void fetchCommandLineInput();
    void pollEvents();

    void runCommandLineLoop();
    void runWindowInputLoop();
    void readEvents(std::ifstream& is, int eventPops);


    void getNextEvent();

    WindowInput windowInput;
    CommandLineInput commandLineInput;
    inline void setInputMode(InputMode mode) {inputMode = mode;}

    EventQueue eventQueue;

private:
	const XWindow& window;
    InputMode inputMode{InputMode::CommandLine};

//    static uint8_t wordCount;
//    static const char** vocabulary;
    inline static Vocab vocabulary{};
    inline static std::vector<const Vocab*> vocabPtrs;
    static char* completion_generator(const char* text, int state);
    static char** completer(const char* text, int start, int end);
    bool keepWindowThread{true};
    bool keepCommandThread{true};

    //pools
    EventPool<CommandLineEvent> clPool{200};
    EventPool<KeyPressEvent> kpPool{50};
    EventPool<KeyReleaseEvent> krPool{20};
    EventPool<MousePressEvent> mpPool{20};
    EventPool<MouseReleaseEvent> mrPool{20};
    EventPool<MouseMotionEvent> mmPool{100};
    EventPool<Abort> aPool{50};
};
