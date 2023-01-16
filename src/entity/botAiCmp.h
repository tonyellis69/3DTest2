#pragma once

#include "aiCmp.h"
#include "../roboState.h"

class CBotAiCmp : public CAiCmp {
public:
	CBotAiCmp(CEntity* parent) : CAiCmp(parent) {}
//	void setState(TRobotState newState, CEntity* entity = NULL);

	std::shared_ptr<CRoboState> currentState = nullptr; ///<The robot's current behaviour.


};
