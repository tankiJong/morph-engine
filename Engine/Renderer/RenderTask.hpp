#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/mat44.hpp"
#include "Engine/Renderer/type.h"

class Camera;
class Mesh;
class Material;

struct RenderTask {
	mat44 model;
  Camera* camera = nullptr;
	const Mesh* mesh = nullptr;
	const Material* material = nullptr;
	uint lightIndices[NUM_MAX_LIGHTS];
	uint lightCount;

	int16  layer;
	uint16 queue;
};
