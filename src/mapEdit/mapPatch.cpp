#include "mapPatch.h"


#include <glm/gtx/transform.hpp> // after <glm/glm.hpp>



CRingPatch::CRingPatch() {
	scale = { radius,radius,0 };
}

void CRingPatch::create() {
	
	CHexElement dummySolidHex;
	dummySolidHex.content = solidHex;
	hexes.clear();

	glm::vec3 v[6];

	matrix = glm::scale(glm::mat4(1), scale);


	for (int n = 0; n < 6; n++) {
		v[n] = cubeToWorldSpace(neighbourHex[n]);
	}

	//transform verts
	for (int n = 0; n < 6; n++) {
		v[n] = (matrix  * glm::vec4(v[n],1.0f) );
	}


	for (int n = 0; n < 6; n++) {
		TIntersections intersectedHexes = getIntersectedHexes(v[n], v[(n + 1) % 6]);
		for (auto& hex : intersectedHexes)
			hexes[hex.first] = dummySolidHex;
	}


}

void CRingPatch::mouseWheel(float delta, int key) {
	glm::vec3 scaleFactor = { 1,1,0 };
	if (key == GLFW_KEY_LEFT_CONTROL)
		scaleFactor = glm::vec3(0, 1, 0);
	else if (key == GLFW_KEY_LEFT_SHIFT)
		scaleFactor = glm::vec3(1, 0, 0);

	if (delta > 0)
		scale += scaleFactor;
	else
		scale -= scaleFactor;

	matrix = glm::scale(glm::mat4(1), scale);

	create();
}

