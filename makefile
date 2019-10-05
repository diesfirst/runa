CFLAGS = -g -std=c++17 -Wall 
LDFLAGS = -lxcb -lvulkan -lX11
DEPS = main.o context.o swapchain.o window.o viewport.o renderer.o description.o io.o mem.o commander.o geo.o occupant.o lodepng.o camera.o 

app : $(DEPS) shaders/*.spv
	g++ $(CFLAGS) -o app $(DEPS) $(LDFLAGS) ; ctags -R .

window.o : window.cpp window.hpp
	g++ -c $(CFLAGS) window.cpp

mem.o : mem.cpp mem.hpp geo.hpp
	g++ -c $(CFLAGS) mem.cpp

io.o : io.cpp io.hpp lib/lodepng.h mem.hpp commander.hpp swapchain.hpp
	g++ -c $(CFLAGS) io.cpp 

event.o : event.cpp event.hpp commander.hpp mem.hpp swapchain.hpp painter.hpp io.hpp util.hpp sculpter.hpp renderer.hpp camera.hpp 
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

geo.o : geo.cpp geo.hpp context.hpp occupant.hpp
	g++ -c $(CFLAGS) geo.cpp

occupant.o : occupant.cpp occupant.hpp 
	g++ -c $(CFLAGS) occupant.cpp

sculpter.o : sculpter.cpp sculpter.hpp geo.hpp
	g++ -c  $(CFLAGS) sculpter.cpp

camera.o : camera.cpp camera.hpp mem.hpp occupant.hpp
	g++ -c $(CFLAGS) camera.cpp

viewport.o : viewport.cpp viewport.hpp swapchain.hpp window.hpp
	g++ -c $(CFLAGS) viewport.cpp

description.o : description.cpp description.hpp context.hpp mem.hpp geo.hpp camera.hpp
	g++ -c $(CFLAGS) description.cpp

renderer.o : renderer.cpp renderer.hpp context.hpp commander.hpp mem.hpp
	g++ -c $(CFLAGS) renderer.cpp

main.o : main.cpp programs.hpp viewport.hpp renderer.hpp description.hpp
	g++ -c $(CFLAGS) main.cpp

.PHONY: clean shaders

clean:
	rm -f *.o .*.swp app

shaders: 
	cd shaders && glslangValidator -V *.glsl
