#pragma once

#include "gridObj.h"

/** Class describing a simple missile object. */
class CBolt : public CGridObj {
public:
	CBolt();
	void fireAt(CHex& target);
	bool update(float dT);

	glm::vec3 targetPos;

};