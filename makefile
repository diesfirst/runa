CFLAGS = -g -std=c++17 -Wall -O0
LDFLAGS = -lxcb -lvulkan -lX11
DEPS = main.o context.o swapchain.o window.o viewport.o renderer.o description.o mem.o commander.o occupant.o lodepng.o util.o io.o

app : $(DEPS) shaders/*.spv
	g++ $(CFLAGS) -o app $(DEPS) $(LDFLAGS) ; ctags -R .

window.o : window.cpp window.hpp
	g++ -c $(CFLAGS) window.cpp

mem.o : mem.cpp mem.hpp 
	g++ -c $(CFLAGS) mem.cpp

io.o : io.cpp io.hpp lib/lodepng.h mem.hpp commander.hpp swapchain.hpp
	g++ -c $(CFLAGS) io.cpp 

event.o : event.cpp event.hpp commander.hpp mem.hpp swapchain.hpp painter.hpp io.hpp util.hpp sculpter.hpp renderer.hpp 
	g++ -c $(CFLAGS) event.cpp

context.o : context.cpp context.hpp commander.hpp mem.hpp
	g++ -c $(CFLAGS) context.cpp

swapchain.o : swapchain.cpp swapchain.hpp context.hpp window.hpp
	g++ -c $(CFLAGS) swapchain.cpp 

painter.o : painter.cpp painter.hpp swapchain.hpp util.hpp
	g++ -c $(CFLAGS) painter.cpp

commander.o : commander.cpp commander.hpp swapchain.hpp painter.hpp
	g++ -c $(CFLAGS) commander.cpp

util.o : util.cpp util.hpp swapchain.hpp context.hpp
	g++ -c $(CFLAGS) util.cpp

lodepng.o : lib/lodepng.cpp lib/lodepng.h
	g++ -c $(CFLAGS) lib/lodepng.cpp

occupant.o : occupant.cpp occupant.hpp 
	g++ -c $(CFLAGS) occupant.cpp

sculpter.o : sculpter.cpp sculpter.hpp 
	g++ -c  $(CFLAGS) sculpter.cpp

viewport.o : viewport.cpp viewport.hpp swapchain.hpp window.hpp
	g++ -c $(CFLAGS) viewport.cpp

description.o : description.cpp description.hpp context.hpp mem.hpp occupant.hpp
	g++ -c $(CFLAGS) description.cpp

renderer.o : renderer.cpp renderer.hpp context.hpp commander.hpp mem.hpp util.hpp
	g++ -c $(CFLAGS) renderer.cpp

main.o : main.cpp programs.hpp viewport.hpp renderer.hpp description.hpp
	g++ -c $(CFLAGS) main.cpp

.PHONY: clean shaders

clean:
	rm -f *.o .*.swp app

shaders: 
	cd shaders && glslangValidator -V *.glsl
