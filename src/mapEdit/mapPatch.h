#pragma once

#define _USE_MATH_DEFINES //for cmath

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

	}
	virtual void create() = 0;
	virtual void resize(float delta, int key) {};
	void rotate(float delta) {
		dir += (delta > 0) ? 1 : -1;
		create();
	}
	virtual void onNewMouseHex(CHex& hex);
	void plotLine(CHex& A, CHex& B);
	void plotVertLine(CHex& A, CHex& B);
	void plotHorizLine(CHex& A, CHex& B);
	void plotDiagLine(CHex& A, CHex& B);
	virtual void setOffset(CHex& offset) {
		this->offset = offset;
	}
	virtual void rotation();


	std::unordered_map<CHex, CHexElement, hex_hash> hexes;
	CHex offset = { 0,0,0 };
	std::vector <glm::vec3> v;
	glm::mat4 matrix; //If I don't need rotations, scrap

	glm::vec2 BB; ///<Bounding box defines  proportions of shape.
	int dir = 0;
};



class CRingPatch: public CMapPatch {
public:
	CRingPatch();
	void create();
	void resize(float delta, int key);
};



class CParagramPatch : public CMapPatch {
public:
	CParagramPatch() { BB = { hexWidth + halfHexWidth, hexRowHeight };
	v.resize(4);
	}
	void create();
	void resize(float delta, int key);

};

class CRectPatch : public CMapPatch {
public:
	CRectPatch() {
		BB = { halfHexWidth, hexRowHeight };
		v.resize(4);
	}
	void create();
	void resize(float delta, int key);
	void rotation();
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

class CTriPatch : public CMapPatch {
public:
	CTriPatch() {
		BB = { hexWidth * 2,2 * hexRowHeight };
		v.resize(3);
	}
	void create();

	void resize(float delta, int key);
	void rotation();
};

#define GLFW_KEY_LEFT_SHIFT         340
#define GLFW_KEY_LEFT_CONTROL       341
