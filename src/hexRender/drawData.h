#pragma once

#include <glm/glm.hpp>

#include "buf2.h"
#include "renderer/mesh.h"

/** Structs for hexRender to batch drawing operations. */

struct TLineDraw {
	CBuf2* buf;
	glm::mat4 matrix;
	std::vector<glm::vec4>* palette;
	TMeshRec* meshRec;
};

struct TSolidDraw {
	CBuf2* buf;
	glm::mat4 matrix;
	std::vector<glm::vec4>* palette;
	TMeshRec* meshRec;
};

struct TSplodeDraw {
	glm::vec3 pos;
	float* lifeTime;
	float* size;
	float* timeOut;
	float* seed;
	int* particleCount;
	std::vector<glm::vec4>* palette;
};