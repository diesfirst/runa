CFLAGS = -O3

app : main.o commander.o swapchain.o painter.o context.o window.o mem.o io.o event.o lodepng.o
	g++ $(CFLAGS) -o app main.o context.o commander.o swapchain.o painter.o window.o mem.o io.o event.o lodepng.o -lxcb -lvulkan -lX11

window.o : window.cpp window.hpp
	g++ -c $(CFLAGS) window.cpp

mem.o : mem.cpp mem.hpp context.hpp
	g++ -c $(CFLAGS) mem.cpp

io.o : io.cpp io.hpp lib/lodepng.h mem.hpp commander.hpp swapchain.hpp
	g++ -c $(CFLAGS) io.cpp 

event.o : event.cpp event.hpp commander.hpp mem.hpp swapchain.hpp painter.hpp io.hpp
	g++ -c $(CFLAGS) event.cpp

context.o : context.cpp context.hpp
	g++ -c $(CFLAGS) context.cpp

swapchain.o : swapchain.cpp swapchain.hpp context.hpp window.hpp
	g++ -c $(CFLAGS) swapchain.cpp 

painter.o : painter.cpp painter.hpp swapchain.hpp
	g++ -c $(CFLAGS) painter.cpp

commander.o : commander.cpp commander.hpp context.hpp swapchain.hpp painter.hpp
	g++ -c $(CFLAGS) commander.cpp

lodepng.o : lib/lodepng.cpp lib/lodepng.h
	g++ -c $(CFLAGS) lib/lodepng.cpp

main.o : main.cpp swapchain.hpp commander.hpp painter.hpp
	g++ -c $(CFLAGS) main.cpp 

.PHONY: clean

clean:
	rm -f *.o app
