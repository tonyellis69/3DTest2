#pragma once

#include "glm/glm.hpp"

#include "../entity/aiCmp.h"

class CEntity;
class CDropAI : public CAiCmp {
public:
	CDropAI(CEntity* drop) : CAiCmp(drop) {}
	void update(float dT);

	glm::vec3 oldPos = glm::vec3(0);
	bool still = false;

};