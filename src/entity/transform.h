#pragma once
#define _USE_MATH_DEFINES

#include <glm/glm.hpp>

#include "hex/hex.h"

#include "component.h"

class CTransformCmp : public CDerivedC<CTransformCmp> {
public:
	CTransformCmp(CEntity* parent) : CDerivedC(parent) {}
	void onAdd();
	void onRemove();
	void setRotation(float angle);
	void setRotation(glm::vec3& vec);
	void rotate(float angle);
	void setUpperBodyRotation(float angle);
	void setUpperBodyRotation(glm::vec3& vec);
	void rotateUpperBody(float angle);
	void setPos(glm::vec3& pos);
	void updatePos(glm::vec3& dPos);
	void setScale(glm::vec3& scale);
	void setWalkTranslation(glm::vec3& walkTranslation);
	void buildWorldMatrix();

	void update(float dT);

	float getRotation();
	glm::vec3 getRotationVec();
	glm::vec3 getUpperBodyRotationVec();
	float getUpperBodyRotation();

	float orientationTo(glm::vec3& targetPos);

	float rotation;
	float upperBodyRotation;
	glm::vec3 worldPos;
	glm::vec3 scale = glm::vec3(1);
	glm::vec3 walk;

	CHex hexPosition = CHex(-1);

	bool upperBodyLocked = true; ///<If true, rotate upper body with base.

};