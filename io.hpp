#include <string>

class MemoryManager;
class Commander;
class Swapchain;

void saveSwapImage(
		MemoryManager& mm,
		Commander& commander,
		Swapchain& swapchain);

std::string requestUserInput();

