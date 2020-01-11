#include "programs.hpp"

int main(int argc, char *argv[])
{
	assert(argc == 2 && "Must provide a card name");
	program3(argv[1]);
	return 0;
}
