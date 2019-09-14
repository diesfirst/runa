CFLAGS = -O3 -std=c++17
LDFLAGS = -lxcb -lvulkan -lX11
DEPS = main.o context.o swapchain.o window.o viewport.o renderer.o

app : $(DEPS)
	g++ $(CFLAGS) -o app $(DEPS) $(LDFLAGS)

window.o : window.cpp window.hpp
	g++ -c $(CFLAGS) window.cpp

mem.o : mem.cpp mem.hpp context.hpp geo.hpp
	g++ -c $(CFLAGS) mem.cpp

io.o : io.cpp io.hpp lib/lodepng.h mem.hpp commander.hpp swapchain.hpp
	g++ -c $(CFLAGS) io.cpp 

event.o : event.cpp event.hpp commander.hpp mem.hpp swapchain.hpp painter.hpp io.hpp util.hpp sculpter.hpp pipe.hpp renderer.hpp camera.hpp 
	g++ -c $(CFLAGS) event.cpp

context.o : context.cpp context.hpp
	g++ -c $(CFLAGS) context.cpp

swapchain.o : swapchain.cpp swapchain.hpp context.hpp window.hpp
	g++ -c $(CFLAGS) swapchain.cpp 

painter.o : painter.cpp painter.hpp swapchain.hpp util.hpp
	g++ -c $(CFLAGS) painter.cpp

commander.o : commander.cpp commander.hpp context.hpp swapchain.hpp painter.hpp
	g++ -c $(CFLAGS) commander.cpp

util.o : util.cpp util.hpp swapchain.hpp context.hpp
	g++ -c $(CFLAGS) util.cpp

lodepng.o : lib/lodepng.cpp lib/lodepng.h
	g++ -c $(CFLAGS) lib/lodepng.cpp

pipe.o : pipe.cpp pipe.hpp context.hpp io.hpp renderer.hpp geo.hpp mem.hpp
	g++ -c $(CFLAGS) pipe.cpp

geo.o : geo.cpp geo.hpp context.hpp
	g++ -c $(CFLAGS) geo.cpp

sculpter.o : sculpter.cpp sculpter.hpp geo.hpp
	g++ -c  $(CFLAGS) sculpter.cpp

camera.o : camera.cpp camera.hpp mem.hpp
	g++ -c $(CFLAGS) camera.cpp

viewport.o : viewport.cpp viewport.hpp swapchain.hpp window.hpp
	g++ -c $(CFLAGS) viewport.cpp

description.o : description.cpp description.hpp context.hpp mem.hpp
	g++ -c $(CFLAGS) description.cpp

renderer.o : renderer.cpp renderer.hpp context.hpp 
	g++ -c $(CFLAGS) renderer.cpp

main.o : main.cpp programs.hpp
	g++ -c $(CFLAGS) main.cpp 

.PHONY: clean shaders

clean:
	rm -f *.o app

shaders: 
	cd shaders && glslangValidator -V *.glsl
