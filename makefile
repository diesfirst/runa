INCDIRS = -isystem /opt/hfs18.5/toolkit/include -isystem /opt/hfs18.5/toolkit/include/python2.7
WFLAGS = -Wno-deprecated -std=c++17 -isystem /opt/hfs18.5/toolkit/include -isystem /opt/hfs18.5/toolkit/include/python2.7 -Wall -W -Wno-parentheses -Wno-sign-compare -Wno-reorder -Wno-uninitialized -Wunused -Wno-unused-parameter -Wno-unused-local-typedefs
CFLAGS = -g -std=c++17 -O2 -D_GLIBCXX_USE_CXX11_ABI=0 $(WFLAGS) $(INCDIRS)
LDFLAGS = -lxcb -lvulkan -lX11 
LDIRS = -L/opt/hfs18.5/dsolib -L/opt/hfs18.5/python/lib -L/usr/X11R6/lib64 -L/usr/X11R6/lib
LUSD = -lHoudiniUSD -lhboost_system -lhboost_python -lpxr_usd -lpxr_tf -lpxr_sdf -lpxr_usdGeom -lpxr_usdImaging -lpxr_hd -lpxr_vt -lpxr_gf -lpython2.7 -lGL -lXext -lXi -ldl -Wl,-rpath,/opt/hfs18.5/dsolib
DEPS = main.o context.o swapchain.o window.o viewport.o renderer.o description.o mem.o commander.o occupant.o lodepng.o util.o io.o

app : $(DEPS) shaders/*.spv
	g++ $(DEPS) -lpthread -o app $(LDIRS) $(LUSD) $(LDFLAGS) ; ctags -R .

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
