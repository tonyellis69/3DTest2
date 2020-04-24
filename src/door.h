#pragma once

#include "gamehextObj.h"

enum TDoorStatus {doorOpening,doorClosing,doorOpen,doorClosed};

/** A hopefully *temporary* class to implement door behaviour. */
class CDoor : public CGameHexObj {
public:
	CDoor();
	void setLineModel(const std::string& name);
	void frameUpdate(float dT);

	TModelNode* doorNode;

	TDoorStatus status;
	float anim;
	float doorSpeed;
};

