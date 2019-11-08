#include "viewport.hpp"
#include "renderer.hpp"
#include "description.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "util.hpp"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usdImaging/usdImaging/sphereAdapter.h>
#include <pxr/imaging/hd/meshTopology.h>
#include <pxr/imaging/hd/meshUtil.h>


Timer myTimer;

void program1()
{
	auto stage = pxr::UsdStage::CreateNew("stage1.usd");

	pxr::SdfPath spherePath("/ball");
	auto ball = stage->DefinePrim(spherePath, pxr::TfToken("Sphere"));

	pxr::UsdImagingSphereAdapter sphereAdapter;
	auto topo = sphereAdapter.GetMeshTopology();
	auto points = sphereAdapter.GetMeshPoints(ball, pxr::UsdTimeCode());
	auto type = topo.GetType();

	if (topo.IsArrayValued())
		std::cout << "The gibbet is array valued, Sir" << std::endl;
	auto meshTopo = topo.Get<pxr::HdMeshTopology>();
	if (points.IsArrayValued())
		std::cout << "The points be array valued!" << std::endl;

	std::cout << "Topo stuff:" << std::endl;
	std::cout << meshTopo.GetNumPoints() << " num points" << std::endl;
	std::cout << meshTopo.GetNumFaceVaryings() << " GetNumFaceVaryings" << std::endl;
	std::cout << meshTopo.GetNumFaces() << " num faces" << std::endl;
	std::cout << "indices" << std::endl;
	auto indices = meshTopo.GetFaceVertexIndices();
	auto faceVertCounts = meshTopo.GetFaceVertexCounts();
	std::cout << indices << std::endl;
	std::cout << "counts" << std::endl;
	std::cout << faceVertCounts << std::endl;

	pxr::HdMeshUtil meshUtil(&meshTopo, spherePath);

	pxr::VtVec3iArray triIndices;
	pxr::VtIntArray primitiveParams;
	meshUtil.ComputeTriangleIndices(&triIndices, &primitiveParams);

	std::cout << "Tri indices" << std::endl;
	std::cout << triIndices << std::endl;
	std::cout << "Prim Params" << std::endl;
	std::cout << primitiveParams << std::endl;
	std::cout << "triIndices size:"
	       << triIndices.size()
	       << "Prim params size:"
	       << primitiveParams.size()
	       << std::endl;


}

void program4()
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
