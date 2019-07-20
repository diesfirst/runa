CFLAGS = -O3

app : main.o commander.o swapchain.o painter.o context.o window.o 
	g++ $(CFLAGS) -o app main.o context.o commander.o swapchain.o painter.o window.o -lxcb -lvulkan

window.o : window.cpp window.hpp
	g++ -c $(CFLAGS) window.cpp

context.o : context.cpp context.hpp
	g++ -c $(CFLAGS) context.cpp

swapchain.o : swapchain.cpp swapchain.hpp context.hpp window.hpp
	g++ -c $(CFLAGS) swapchain.cpp 

painter.o : painter.cpp painter.hpp swapchain.hpp
	g++ -c $(CFLAGS) painter.cpp

commander.o : commander.cpp commander.hpp context.hpp swapchain.hpp painter.hpp
	g++ -c $(CFLAGS) commander.cpp

main.o : main.cpp swapchain.hpp commander.hpp painter.hpp
	g++ -c $(CFLAGS) main.cpp 

.PHONY: clean

clean:
	rm -f *.o app
