#pragma once

#include <glm/glm.hpp>

#include "component.h"

class CTransformCmp : public CEntityCmp {
public:
	CTransformCmp(CEntity* parent) : CEntityCmp(parent) {}
	void setRotation(float angle);
	void setPos(glm::vec3& pos);
	void setPosOffGrid(glm::vec3& pos);
	void setScale(glm::vec3& scale);
	void buildWorldMatrix();

	void update(float dT);

};