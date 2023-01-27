#pragma once

#include "../entity/aiCmp.h"

class CEntity;
class CDropAI : public CAiCmp {
public:
	CDropAI(CEntity* drop) : CAiCmp(drop) {}
	void update(float dT);

};