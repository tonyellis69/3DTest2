#include "mapPatch.h"

#include "utils/log.h"

constexpr auto TAN30 = 0.57735026918962576450914878050196;


void CMapPatch::onNewMouseHex(CHex& hex) {
	offset = hex;
}

/** Draws a more aesthetically pleasing line without redundant hexes at the ends. */
void CMapPatch::plotVertLine(CHex& A, CHex& B) {
	CHex top = A; CHex bot = B;
	if (A.z > B.z) {
		top = B; bot = A;
	}

	bool veerEast = (bot.x >= 0) ? true : false;
	CHex hex = bot;
	for (int z = bot.z; z > top.z; z--) {
		if (veerEast)
			hex = getNeighbour(hex, hexNE);
		else
			hex = getNeighbour(hex, hexNW);
		veerEast = !veerEast;
		hexes[hex].content = solidHex;
	}
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
	int hDist = abs(A.z) + abs(B.z);
	CHex hex = A;
	for (int n = 0; n < hDist; n++ ) {
		hexes[hex].content = solidHex;
		hex = getNeighbour(hex, dir);
	}
}




CRingPatch::CRingPatch() {
	scale = { radius,radius,0 };
	BB = { hexWidth, hexRowHeight };
}

void CRingPatch::create() {
	v.resize(6);

	matrix = glm::scale(glm::mat4(1), scale);

	//calculate verts
	//float nearX = BB.x - (BB.y / hexRowHeight) * halfHexWidth;
	float nearX = BB.x - (TAN30 * BB.y);
	v[0] = { nearX, BB.y,0 };
	v[1] = { BB.x, 0,0 };
	v[2] = { nearX, -BB.y,0 };
	v[3] = {-nearX, -BB.y,0 };
	v[4] = { -BB.x, 0,0 };
	v[5] = { -nearX, BB.y,0 };


	//transform verts
	for (int n = 0; n < 6; n++) {
		v[n] = (matrix  * glm::vec4(v[n],1.0f) );
	}

	hexes.clear();
	plotDiagLine(worldSpaceToHex(v[0]), worldSpaceToHex(v[1]));
	plotDiagLine(worldSpaceToHex(v[1]), worldSpaceToHex(v[2]));
	plotHorizLine(worldSpaceToHex(v[2]), worldSpaceToHex(v[3]));
	plotDiagLine(worldSpaceToHex(v[3]), worldSpaceToHex(v[4]));
	plotDiagLine(worldSpaceToHex(v[4]), worldSpaceToHex(v[5]));
	plotHorizLine(worldSpaceToHex(v[5]), worldSpaceToHex(v[0]));
}

void CRingPatch::mouseWheel(float delta, int key){
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
	v.resize(4);

	matrix = glm::scale(glm::mat4(1), scale);


	float nearX = (BB.y * 4/3);
	float adj = (TAN30 * BB.y * 2) ;
	nearX = BB.x - adj;// nearX;
	v[0] = { BB.x, BB.y,0 };
	v[1] = { nearX, -BB.y,0 };
	v[2] = {-BB.x, -BB.y,0 };
	v[3] = { -nearX, BB.y,0 };

	//transform verts
	for (int n = 0; n < 4; n++) {
		v[n] = (matrix * glm::vec4(v[n], 1.0f));
	}

	hexes.clear();
	plotDiagLine(worldSpaceToHex(v[0]), worldSpaceToHex(v[1]));
	plotHorizLine(worldSpaceToHex(v[1]), worldSpaceToHex(v[2]));
	plotDiagLine(worldSpaceToHex(v[2]), worldSpaceToHex(v[3]));
	plotHorizLine(worldSpaceToHex(v[3]), worldSpaceToHex(v[0]));
}

void CParagramPatch::mouseWheel(float delta, int key) {
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
	v.resize(4);

	if (glm::length(scale) == 0)
		scale = { 1,1,0 };

	matrix = glm::scale(glm::mat4(1), scale);


	v[0] = { BB.x,BB.y,0 };
	v[1] = { BB.x,-BB.y,0 };
	v[2] = { -BB.x,-BB.y,0 };
	v[3] = {-BB.x,BB.y,0 };


	//transform verts
	for (int n = 0; n < 4; n++) {
		v[n] = (matrix * glm::vec4(v[n], 1.0f));
	}

	hexes.clear();
	plotVertLine(worldSpaceToHex(v[0]), worldSpaceToHex(v[1]));
	plotHorizLine(worldSpaceToHex(v[1]), worldSpaceToHex(v[2]));
	plotVertLine(worldSpaceToHex(v[3]), worldSpaceToHex(v[2]));
	plotHorizLine(worldSpaceToHex(v[3]), worldSpaceToHex(v[0]));
}


void CRectPatch::mouseWheel(float delta, int key) {
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

CLinePatch::CLinePatch(CHex& startHex) {
	this->startHex = startHex;
	endHex = startHex;
	v.resize(2);
}

void CLinePatch::create() {

	matrix = glm::scale(glm::mat4(1), scale);


	v[0] = cubeToWorldSpace(startHex);
	v[1] = cubeToWorldSpace(endHex);

	//transform verts
	//for (int n = 0; n < 2; n++) {
	//	v[n] = (matrix * glm::vec4(v[n], 1.0f));
	//}

	hexes.clear();
	hexes[startHex].content = solidHex;
	//TIntersections hexList = getIntersectedHexes(v[0],v[1]);
	//for (auto& hex : hexList) {
	//	hexes[hex.first].content = solidHex;
	//}
	THexList hexList = *hexLine(startHex, endHex);
	for (auto& hex : hexList)
		hexes[hex].content = solidHex;
}

void CLinePatch::onNewMouseHex(CHex& hex) {
	endHex = hex;
	create();
}
