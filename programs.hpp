#include "viewport.hpp"
#include "renderer.hpp"
#include "description.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "util.hpp"


Timer myTimer;

void program0()
{
	Context context;
	Viewport viewport(context);
	Renderer renderer(context);
	Description scene(context);
	renderer.setup(viewport, scene);

	auto quad = scene.createMesh<Quad>();

	constexpr int waitTime = 1000;
	renderer.update();
	renderer.render();
	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
	quad->rotate(.1, {0, 0, 1});
	std::cout << "about to render again" << std::endl;
	renderer.render();
	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
	quad->rotate(.1, {0, 0, 1});
	renderer.render();
	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
	quad->rotate(.1, {0, 0, 1});
	renderer.render();
	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
	quad->rotate(.1, {0, 0, 1});
	renderer.render();
	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
	quad->rotate(.1, {0, 0, 1});
	renderer.render();
	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
	quad->rotate(.1, {0, 0, 1});
	renderer.render();
	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
	context.queue.waitIdle();
}

void textureTest()
{
	Context context;
	Viewport viewport(context);
	Renderer renderer(context);
	Description scene(context);
	renderer.setup(viewport, scene);

	scene.loadImage("./images/sunflower.jpg");

}

void program1()
{
	Context context;
	std::cout << "yeet" << std::endl;
	Viewport myViewport(context);
	Renderer ren(context);
	Description scene(context);

	auto tri1 = scene.createMesh<Triangle>();
	tri1->translate(-0.5, 0, 0);
	tri1->scale(0.2);
	std::cout << "This the memory address of tri1: " << tri1 << std::endl;

	ren.setup(myViewport, scene);
	ren.update();
	ren.render();

	std::cout << "Choose a point" << std::endl;
	uint32_t index;
	std::cin >> index;
	auto p0 = tri1->getPoint(index);

	std::cout << "Choose a color" << std::endl;
	float r,g,b;
	std::cin >> r;
	std::cin >> g;
	std::cin >> b;
	p0->setColor(r,g,b);
	scene.update(tri1);
	ren.render();
	std::this_thread::sleep_for(std::chrono::seconds(4));

	context.queue.waitIdle();
}
