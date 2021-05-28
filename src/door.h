#pragma once

#include "entity.h"

enum TDoorStatus {doorOpening,doorClosing,doorOpen,doorClosed};

/** A hopefully *temporary* class to implement door behaviour. */
class CDoor : public CEntity {
public:
	CDoor();
	void setLineModel(const std::string& name);
	void frameUpdate(float dT);

	unsigned int blocks();

	TModelNode* doorNode;

	TDoorStatus status;
	float anim;
	float doorSpeed;
};

