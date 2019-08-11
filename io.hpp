#include <string>
#include <fstream>
#include <vector>

class MemoryManager;
class Commander;
class Swapchain;

namespace io 
{
	void saveSwapImage(
			MemoryManager& mm,
			Commander& commander,
			Swapchain& swapchain);

	std::vector<char> readFile(const std::string& filename);

	std::string requestUserInput();
}

