#include "entity.h"

#include "utils\log.h"

#include "..\3Dtest\src\hexRenderer.h"

#include <glm/gtc/matrix_transform.hpp>	

#include "gameState.h"

const float rad360 = M_PI * 2;


CEntity::CEntity() {

}


void CEntity::setModel(TModelData& model) {
	lineModel.model = model;
	setBoundingRadius();
}

/** Set position using hex cube coordinates. */
void CEntity::setPosition(CHex& hex) {
	setPosition(cubeToWorldSpace(hex));
}
//TO DO: phase out the old hex-based usage!

void CEntity::setPosition(glm::vec3& worldPos) {
	hexPosition = worldSpaceToHex(worldPos);
	destination = hexPosition;

	glm::i32vec2 axial = hexPosition.getAxial();

	this->worldPos = worldPos;

	buildWorldMatrix();
}

void CEntity::setBoundingRadius() {
	physics.boundingRadius = glm::length(lineModel.model.extents.furthestVert);
}

/** Set the rotation and facing direction of this object. */
void CEntity::setHexDirection(THexDir direction) {
	facing = direction;
	setRotation(dirToAngle(direction));
	buildWorldMatrix();
}

/** Set our rotation to this absolute. */
void CEntity::setRotation(float angle) {
	rotation = angle;
}

/** Rotated by the given amount.*/
void CEntity::rotate(float angle) {
	rotation += angle;
	rotation = fmod(rotation + rad360, rad360);
}


void CEntity::draw(){
	if (hexPosition == CHex(-1))
		return;
	hexRendr2.drawLineModel(lineModel);
}


/** Construct this object's world matrix from its known position and rotation.*/
void CEntity::buildWorldMatrix() {
	lineModel.model.matrix = glm::translate(glm::mat4(1), worldPos);
	lineModel.model.matrix = glm::rotate(lineModel.model.matrix, rotation, glm::vec3(0, 0, -1));
	//NB: we use a CW system for angles

	//updateMatrices(lineModel.model);

	for (auto& childModel : lineModel.model.subModels)
		updateMatrices(childModel);
}

void CEntity::updateMatrices(TModelData& model) {
	model.matrix = glm::translate(glm::mat4(1), worldPos);

	for (auto& childModel : model.subModels)
		updateMatrices(childModel);
}

std::tuple<float, glm::vec3> CEntity::collisionCheck(CEntity* e2) {
	//get bounding-sphere radii
	float radius1 = lineModel.getRadius();
	float radius2 = e2->lineModel.getRadius();
	
	//check for overlap
	float entDist = glm::distance(worldPos, e2->worldPos);
	if (entDist > radius1 + radius2)
		return { 0, {0,0,0} };
	else {
		float collisionDist = entDist - radius2;
		glm::vec3 collisionNormal = glm::normalize(e2->worldPos - worldPos);
		return { collisionDist, collisionNormal};
	}

}

/** Return shortest angle of rotation between the way we're facing and the given point. */
float CEntity::orientationTo(glm::vec3& targetPos) {
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

/** Modify worldspace position by the given vector. */
void CEntity::updatePos(glm::vec3& dPos) {
	worldPos += dPos;
//	buildWorldMatrix();
	CHex newHexPosition = worldSpaceToHex(worldPos);
	if (newHexPosition != hexPosition) {
		//world.map->movedTo(this, hexPosition, newHexPosition);
		hexPosition = newHexPosition;
		onMovedHex();
	}
}

/** True if entity on screen. */
bool CEntity::isOnScreen() {
	return hexRendr2.isOnScreen(worldPos);
}


