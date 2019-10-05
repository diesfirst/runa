#include "programs.hpp"

int main(int argc, char *argv[])
{
	if (argc == 2) {
		std::cout << "Running " << argv[0] << std::endl;
		program2(std::atoi(argv[1]));
	}
	else {
		program1();
	}
	return 0;
}
