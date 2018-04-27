#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/mat44.hpp"
#include "Engine/Renderer/type.h"

class Camera;
class Mesh;
class Material;
class Transform;
struct RenderTask {
  Camera* camera = nullptr;
	const Transform* transform = nullptr;
	const Mesh* mesh = nullptr;
	const Material* material = nullptr;
	uint lightIndices[NUM_MAX_LIGHTS];
	uint lightCount;

//	uint layer = 0;
//	uint queue = 0;

	static void sort(const Camera& cam, span<RenderTask> tasks);
};
