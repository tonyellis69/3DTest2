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
	virtual void mouseWheel(float delta, int key) {};
	virtual void onNewMouseHex(CHex& hex);
	void plotVertLine(CHex& A, CHex& B);
	void plotHorizLine(CHex& A, CHex& B);
	void plotDiagLine(CHex& A, CHex& B);
	virtual void setOffset(CHex& offset) {
		this->offset = offset;
	}


	std::unordered_map<CHex, CHexElement, hex_hash> hexes;
	CHex offset = { 0,0,0 };
	glm::vec3 scale = { 1,1,0 };
	std::vector <glm::vec3> v;
	glm::mat4 matrix;


	glm::vec2 BB; ///<Bounding box defines  proportions of shape.
};



class CRingPatch: public CMapPatch {
public:
	CRingPatch();
	void create();
	void mouseWheel(float delta, int key);

	int radius = 1;

};



class CParagramPatch : public CMapPatch {
public:
	CParagramPatch() { scale = { 1,1,0 }; 	BB = { hexWidth + halfHexWidth, hexRowHeight };
	}
	void create();
	void mouseWheel(float delta, int key);

};

class CRectPatch : public CMapPatch {
public:
	CRectPatch() {
		scale = { 1,1,0 };
		BB = { halfHexWidth, hexRowHeight };
	}
	void create();
	void mouseWheel(float delta, int key);
};


class CLinePatch : public CMapPatch {
public:
	CLinePatch() {};
	CLinePatch(CHex& startHex);
	void create();
	void onNewMouseHex(CHex& hex);
	void setOffset(CHex& offset) {}

	CHex startHex;
	CHex endHex;
};

#define GLFW_KEY_LEFT_SHIFT         340
#define GLFW_KEY_LEFT_CONTROL       341