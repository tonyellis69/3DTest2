#include "mapPatch.h"


#include <cmath>
#include <glm/gtx/rotate_vector.hpp>

#include "utils/log.h"

constexpr auto TAN30 = 0.57735026918962576450914878050196;
constexpr float rad60 = M_PI / 3.0f;
constexpr float rad90 = M_PI / 2.0f;


void CMapPatch::onNewMouseHex(CHex& hex) {
	offset = hex;
}

void CMapPatch::plotLine(CHex& A, CHex& B) {
	if (A.z == B.z)
		plotHorizLine(A, B);
	else if (A.x == B.x || A.y == B.y)
		plotDiagLine(A, B);
	else
		plotVertLine(A, B);
}

/** Draws a more aesthetically pleasing line without redundant hexes at the ends. */
void CMapPatch::plotVertLine(CHex& A, CHex& B) {
	bool veerEast = (A.x > 0) ? true : false;

	THexList hexList = vertLine(A, B, veerEast);
	for (auto& hex : hexList)
		hexes[hex].content = solidHex;
}


void CMapPatch::plotHorizLine(CHex& A, CHex& B) {
	if (A == B)
		return;
	THexDir dir = relativeDir(A, B);
	for (CHex hex = A; hex != getNeighbour(B, dir); ) {
		hexes[hex].content = solidHex;
		hex = getNeighbour(hex, dir);
	}
}

void CMapPatch::plotDiagLine(CHex& A, CHex& B) {
	if (A == B)
		return;
	THexDir dir = relativeDir(A, B);
	int hDist = abs(A.z - B.z);
	CHex hex = A;
	for (int n = 0; n < hDist; n++ ) {
		hexes[hex].content = solidHex;
		hex = getNeighbour(hex, dir);
	}
}

void CMapPatch::rotation() {
	dir = dir % 6;
	if (dir != 0) {
		matrix = glm::rotate(rad60 * dir, glm::vec3(0, 0, 1));
		for (auto& vert : v)
			vert = matrix * glm::vec4(vert, 1);
	}

}




CRingPatch::CRingPatch() {
	BB = { hexWidth, hexRowHeight };
	v.resize(6);
}

void CRingPatch::create() {

	//calculate verts
	float nearX = BB.x - (TAN30 * BB.y);
	v[0] = { nearX, BB.y,0 };
	v[1] = { BB.x, 0,0 };
	v[2] = { nearX, -BB.y,0 };
	v[3] = {-nearX, -BB.y,0 };
	v[4] = { -BB.x, 0,0 };
	v[5] = { -nearX, BB.y,0 };

	rotation();

	hexes.clear();

	plotLine(worldSpaceToHex(v[0]), worldSpaceToHex(v[1]));
	plotLine(worldSpaceToHex(v[1]), worldSpaceToHex(v[2]));
	plotLine(worldSpaceToHex(v[2]), worldSpaceToHex(v[3]));
	plotLine(worldSpaceToHex(v[3]), worldSpaceToHex(v[4]));
	plotLine(worldSpaceToHex(v[4]), worldSpaceToHex(v[5]));
	plotLine(worldSpaceToHex(v[5]), worldSpaceToHex(v[0]));
}

void CRingPatch::resize(float delta, int key){
	if (key == GLFW_KEY_LEFT_CONTROL) {
		BB.x += delta * hexWidth;
	}
	else if (key == GLFW_KEY_LEFT_SHIFT) {
		BB.y += delta * hexRowHeight;
	}
	else {
		BB.x += delta * hexWidth;
		BB.y += delta * hexRowHeight;
	}

	create();
}



/** Patch to create a parallelogram. */
void CParagramPatch::create() {
	float nearX = (BB.y * 4/3);
	float adj = (TAN30 * BB.y * 2) ;
	nearX = BB.x - adj;
	v[0] = { BB.x, BB.y,0 };
	v[1] = { nearX, -BB.y,0 };
	v[2] = {-BB.x, -BB.y,0 };
	v[3] = { -nearX, BB.y,0 };

	hexes.clear();
	rotation();

	plotLine(worldSpaceToHex(v[0]), worldSpaceToHex(v[1]));
	plotLine(worldSpaceToHex(v[1]), worldSpaceToHex(v[2]));
	plotLine(worldSpaceToHex(v[2]), worldSpaceToHex(v[3]));
	plotLine(worldSpaceToHex(v[3]), worldSpaceToHex(v[0]));
}

void CParagramPatch::resize(float delta, int key) {
	if (key == GLFW_KEY_LEFT_CONTROL) {
		BB.x += delta * hexWidth;
	}
	else if (key == GLFW_KEY_LEFT_SHIFT) {
		BB.y += delta * hexRowHeight;
		BB.x += delta * (halfHexWidth);
	}
	else {
		BB.x += delta * (hexWidth + halfHexWidth);
		BB.y += delta * hexRowHeight ;
	}

	create();
}

void CRectPatch::create() {
	v[0] = { BB.x,BB.y,0 };
	v[1] = { BB.x,-BB.y,0 };
	v[2] = { -BB.x,-BB.y,0 };
	v[3] = {-BB.x,BB.y,0 };

	rotation();
	hexes.clear();
	plotLine(worldSpaceToHex(v[0]), worldSpaceToHex(v[1]));
	plotLine(worldSpaceToHex(v[1]), worldSpaceToHex(v[2]));
	plotLine(worldSpaceToHex(v[3]), worldSpaceToHex(v[2]));
	plotLine(worldSpaceToHex(v[3]), worldSpaceToHex(v[0]));
}


void CRectPatch::resize(float delta, int key) {
	if (key == GLFW_KEY_LEFT_CONTROL) {
		BB.x += delta * hexWidth;
	}
	else if (key == GLFW_KEY_LEFT_SHIFT) {
		BB.y += delta * hexRowHeight;
	}
	else {
		BB.x += delta * (hexWidth );
		BB.y += delta * hexRowHeight;
	}

	create();
}

void CRectPatch::rotation() {
	dir = dir % 6;
	if (dir != 0) {
		matrix = glm::rotate(rad90 * dir, glm::vec3(0, 0, 1));
		for (auto& vert : v)
			vert = matrix * glm::vec4(vert, 1);
	}
}

CLinePatch::CLinePatch(CHex& startHex) {
	this->startHex = startHex;
	endHex = startHex;
	v.resize(2);
}

void CLinePatch::create() {

	v[0] = cubeToWorldSpace(startHex);
	v[1] = cubeToWorldSpace(endHex);

	hexes.clear();
	hexes[startHex].content = solidHex;

	THexList hexList = *hexLine(startHex, endHex);
	for (auto& hex : hexList)
		hexes[hex].content = solidHex;
}

void CLinePatch::onNewMouseHex(CHex& hex) {
	endHex = hex;
	create();
}

void CTriPatch::create() {
	v[0] = {0 ,BB.y,0 };
	v[1] = { BB.x,-BB.y,0 };
	v[2] = { -BB.x,-BB.y,0 };

	rotation();

	hexes.clear();
	plotLine(worldSpaceToHex(v[0]), worldSpaceToHex(v[1]));
	plotLine(worldSpaceToHex(v[1]), worldSpaceToHex(v[2]));
	plotLine(worldSpaceToHex(v[2]), worldSpaceToHex(v[0]));
}

void CTriPatch::resize(float delta, int key) {

	BB.x += delta * (hexWidth * 2);
	BB.y += delta * (hexRowHeight * 2);
	

	create();
}

void CTriPatch::rotation() {
	dir = dir % 6;
	if (dir != 0) {
		matrix = glm::rotate(float(M_PI) * dir, glm::vec3(0, 0, 1));
		for (auto& vert : v)
			vert = matrix * glm::vec4(vert, 1);
	}
}



void CTrapPatch::create() {
	float nearX;

	float adj = (TAN30 * BB.y * 2);
	nearX = BB.x - adj;


	v[0] = { -nearX ,BB.y,0 };
	v[1] = { nearX,BB.y,0 };
	v[2] = { BB.x,-BB.y,0 };
	v[3] = { -BB.x,-BB.y,0 };

	rotation();

	hexes.clear();
	plotLine(worldSpaceToHex(v[0]), worldSpaceToHex(v[1]));
	plotLine(worldSpaceToHex(v[1]), worldSpaceToHex(v[2]));
	plotLine(worldSpaceToHex(v[2]), worldSpaceToHex(v[3]));
	plotLine(worldSpaceToHex(v[3]), worldSpaceToHex(v[0]));
}

void CTrapPatch::resize(float delta, int key) {
	if (key == GLFW_KEY_LEFT_CONTROL) {
		BB.x += delta * hexWidth;
	}
	else if (key == GLFW_KEY_LEFT_SHIFT) {
		BB.y += delta * hexRowHeight;
	}
	else {
		BB.x += delta * (hexWidth + halfHexWidth);
		BB.y += delta * (hexRowHeight);
	}

	create();
}

void CTrapPatch::rotation() {

}




void CDeleteRect::create() {
	glm::vec3 wsBL = cubeToWorldSpace(BR);
	v[0] = cubeToWorldSpace(TL); 
	v[1] = glm::vec3(wsBL.x,v[0].y,0 );
	v[2] = wsBL;
	v[3] = glm::vec3(v[0].x, wsBL.y, 0);

	rotation();
	hexes.clear();
	plotLine(worldSpaceToHex(v[0]), worldSpaceToHex(v[1]));
	plotLine(worldSpaceToHex(v[1]), worldSpaceToHex(v[2]));
	plotLine(worldSpaceToHex(v[2]), worldSpaceToHex(v[3]));
	plotLine(worldSpaceToHex(v[3]), worldSpaceToHex(v[0]));
}

void CDeleteRect::drag(CHex& newPos) {
	CHex localNewPos = newPos - offset;
	if (localNewPos + neighbourHex[hexSE] !=  BR /* + neighbourHex[hexNW]*/) {
		BR = localNewPos + neighbourHex[hexSE];
		create();
	}
}

void CDeleteRect::findDelHexes() {
	glm::i32vec2 TLoff = cubeToOffset(TL);
	glm::i32vec2 BRoff = cubeToOffset(BR);

	int yDist = abs(BRoff.y - TLoff.y);
	int xDist = abs(BRoff.x - TLoff.x);


	hexes.clear();
	for (int y = 0; y <= yDist; y++) {
		for (int x = 0; x <= xDist; x++) {
			//delHexes.push_back(offsetToCube(TLoff.x + x, TLoff.y + y));
			hexes[offsetToCube(TLoff.x + x, TLoff.y + y)].content = solidHex;
			//CHex topHex = offsetToCube(TLoff.x + x, TLoff.y);
			//CHex botHex = offsetToCube(TLoff.x + x, BRoff.y+1);
			//plotVertLine(topHex,botHex);
		}
	}


}
