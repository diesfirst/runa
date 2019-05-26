#include <iostream>
#include "window.hpp"

int main(int argc, char *argv[])
{
	Window window;
	window.open();
	std::cout << "Still running..." << std::endl;
	return 0;
}
