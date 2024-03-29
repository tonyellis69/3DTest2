#include "transform.h"



#include <cmath>
#include <glm/gtc/matrix_transform.hpp>	
#include <glm/gtx/vector_angle.hpp> 

#include "utils/mathsLib.h"

#include "entity.h"

//const float rad360 = float(M_PI) * 2.0f;

void CTransformCmp::onAdd() {
	thisEntity->transform = this;
}

void CTransformCmp::onRemove() {
	thisEntity->transform = nullptr;
}

void CTransformCmp::setRotation(float angle) {
	if (upperBodyLocked) {
		float diff = upperBodyRotation - rotation;
		upperBodyRotation = angle + diff;
		upperBodyRotation = fmod(upperBodyRotation + rad360, rad360);
	}

	//thisEntity->modelCmp->rotate(angle); ///Temp remove!!!
	rotation = angle;
}

void CTransformCmp::setRotation(glm::vec3& vec) {
	float angle = glm::orientedAngle(vec, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
	setRotation(angle);
}

void CTransformCmp::rotate(float angle) {
	rotation += angle;
	rotation = fmod(rotation + rad360, rad360);
	if (upperBodyLocked) {
		upperBodyRotation += angle;
		upperBodyRotation = fmod(upperBodyRotation + rad360, rad360);
	}
}

void CTransformCmp::setUpperBodyRotation(float angle)
{
	upperBodyRotation = angle;
}

void CTransformCmp::setUpperBodyRotation(glm::vec3& vec)
{
	upperBodyRotation = glm::orientedAngle(vec, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
}

void CTransformCmp::rotateUpperBody(float angle) {
	upperBodyRotation += angle;
	upperBodyRotation = fmod(upperBodyRotation + rad360, rad360);
}

void CTransformCmp::setPos(glm::vec3& pos) {
	//thisEntity->modelCmp->translateAll(pos);
	worldPos = pos;
	hexPosition = worldSpaceToHex(worldPos);
}


/** Modify position by the given vector. */
void CTransformCmp::updatePos(glm::vec3& dPos) {
	glm::vec3 oldWorldPos = worldPos;
	worldPos += dPos;
	//thisEntity->worldPos = worldPos; //temp until worldPos banished

	CHex newHexPosition = worldSpaceToHex(worldPos);
	if (newHexPosition != hexPosition) {
		hexPosition = newHexPosition;
		//thisEntity->transform->hexPosition = newHexPosition;
	}
}

void CTransformCmp::setScale(glm::vec3& scaleVec) {
	//thisEntity->modelCmp->scale(scaleVec);
	scale = scaleVec;
}

void CTransformCmp::setWalkTranslation(glm::vec3& walkTranslation)
{
	walk = walkTranslation;
}

void CTransformCmp::buildWorldMatrix() {

//never get here
}

void CTransformCmp::update(float dT) {
	buildWorldMatrix();
}

float CTransformCmp::getRotation()
{
	return rotation;
}

glm::vec3 CTransformCmp::getRotationVec()
{
	return { cos(rotation), -sin(rotation),0 };
}

glm::vec3 CTransformCmp::getUpperBodyRotationVec()
{
	return  { cos(upperBodyRotation), -sin(upperBodyRotation),0 };
}

float CTransformCmp::getUpperBodyRotation()
{
	return upperBodyRotation;
}

/** Return shortest angle of rotation between the way we're facing and the given point. */
float CTransformCmp::orientationTo(glm::vec3& targetPos) {
	//find direction to target
	glm::vec3 targetDir = targetPos - worldPos;
	targetDir = glm::normalize(targetDir);
	float targetAngle = glm::acos(glm::dot(targetDir, glm::vec3(1, 0, 0)));

	//convert from [0 - pi] angle to [0 - 2pi], ie full circle
	if (targetDir.y > 0)
		targetAngle = 2 * M_PI - targetAngle;

	//NB measuring angle clockwise


	float PI_2 = 2 * M_PI;
	//find shortest angle between this and our direction
	float dist = fmod(PI_2 + targetAngle - rotation, PI_2);

	//put in range [-pi - pi] to give angle a direction, ie, clockwise/anti
	if (dist > M_PI)
		dist = -(PI_2 - dist);

	return dist;
}
