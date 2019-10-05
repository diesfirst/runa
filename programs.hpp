#include "viewport.hpp"
#include "renderer.hpp"
#include "description.hpp"
#include <thread>
#include <chrono>
#include <random>

void triangleFun(Description& desc, uint32_t count)
{
	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_real_distribution<float> uniform_dist(-1.0, 1.0);
	std::uniform_real_distribution<float> uniform_color(0.0, 1.0);
	for (uint32_t i = 0; i < count; ++i) {
		std::array<Point, 3> points;
		glm::vec3 color(
				uniform_color(e1),
				uniform_color(e1),
				uniform_color(e1));
		for (int i = 0; i < 3; ++i) {
			points[i].pos.x = uniform_dist(e1);
			points[i].pos.y = uniform_dist(e1);
			points[i].pos.z = uniform_dist(e1);
			points[i].color = color;
			points[i].color.r = uniform_color(e1);
			points[i].color.g = uniform_color(e1);
			points[i].color.b = uniform_color(e1);
		}
		desc.createTriangle(points[0], points[1], points[2]);
	}
}

void program1()
{
	Context context;
	Viewport viewport(context);
	Renderer renderer(context);
	Description description(context);
	//binds to viewport and description and creates the graphics Pipeline
	Triangle* tri1 = description.createTriangle();
	Triangle* tri2 = description.createTriangle();
	tri1->scale(0.2);
	tri2->translate(-.1, .3, 0);
	tri2->scale(0.2);
	tri2->rotate(3.14, glm::vec3(0, 0, 1));
	renderer.setup(viewport, description); 
	//renderer must update after new geo is created
	renderer.update();
	renderer.render();
	std::this_thread::sleep_for(std::chrono::seconds(2));
	tri2->rotate(1, glm::vec3(0, 0, 1));
	renderer.render();
	std::cout << "frame 2" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(2));

	float angle1 = 0.1;
	float angle2 = -0.12;
	glm::vec3 axis(0, 0, 1);
	while (true)
	{
		tri2->rotate(angle1, axis);
		tri1->rotate(angle2, axis);
		renderer.render();
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
	context.queue.waitIdle();
}

void program2(int count)
{
	Context context;
	Viewport viewport(context);
	Renderer renderer(context);
	Description description(context);
	//binds to viewport and description and creates the graphics Pipeline
	renderer.setup(viewport, description); 
	triangleFun(description, count);
	//renderer must update after new geo is created
	renderer.update();
	renderer.render();
	context.queue.waitIdle();
	std::this_thread::sleep_for(std::chrono::seconds(4));
}
