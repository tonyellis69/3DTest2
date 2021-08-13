#pragma once

#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "hex/hex.h"
#include "hex/hexElement.h"

#include <glm/gtx/transform.hpp> // after <glm/glm.hpp>

using THexes = std::pair<CHex, CHexElement>;

/** Encapsulates a map addition. */
class CMapPatch {
public:
	CMapPatch() {
		matrix = glm::scale(glm::mat4(1), scale);
	}
	virtual void create() = 0;
	virtual void mouseWheel(float delta, int key) = 0;

	//std::vector<THexes> hexes;
	std::unordered_map<CHex, CHexElement, hex_hash> hexes;
	CHex offset = { 0,0,0 };
	glm::vec3 scale = { 1,1,0 };

	glm::mat4 matrix;
};



class CRingPatch: public CMapPatch {
public:
	CRingPatch();
	void create();
	void mouseWheel(float delta, int key);

	int radius = 4;

};

#define GLFW_KEY_LEFT_SHIFT         340
#define GLFW_KEY_LEFT_CONTROL       341