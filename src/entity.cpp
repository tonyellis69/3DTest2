#include "entity.h"

#include "utils\log.h"

#include "..\3Dtest\src\hexRenderer.h"

#include <glm/gtc/matrix_transform.hpp>	

#include "gameState.h"

CEntity::CEntity() {
	worldMatrix = &lineModel.model.matrix;
	//TO DO: ugh fix. Maybe with components
}


/** Set position using hex cube coordinates. */
void CEntity::setPosition(CHex& hex) {
	hexPosition = hex;
	destination = hexPosition;

	glm::i32vec2 axial = hexPosition.getAxial();

	worldPos.x = hexWidth * axial.x + hexWidth / 2.0f * axial.y;
	worldPos.y = -3.0f / 2.0f * axial.y;
	worldPos.z = 0;

	buildWorldMatrix();
}

void CEntity::setLineModel(const std::string& name) {
	lineModel = hexRendr2.getLineModel(name);
	physics.boundingRadius = glm::length(lineModel.model.extents.furthestVert);
}

/** Set the rotation and facing direction of this object. */
void CEntity::setHexDirection(THexDir direction) {
	facing = direction;
	rotation = dirToAngle(direction);
	buildWorldMatrix();
}


void CEntity::draw(){
	if (hexPosition == CHex(-1))
		return;
	hexRendr2.drawLineModel(lineModel);
}


/** Construct this object's world matrix from its known position and rotation.*/
void CEntity::buildWorldMatrix() {
	*worldMatrix = glm::translate(glm::mat4(1), worldPos);
	*worldMatrix = glm::rotate(*worldMatrix, rotation, glm::vec3(0, 0, -1));
	//NB: we use a CW system for angles
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
	buildWorldMatrix();
	CHex newHexPosition = worldSpaceToHex(worldPos);
	if (newHexPosition != hexPosition) {
		world.map->movedTo(this, hexPosition, newHexPosition);
		hexPosition = newHexPosition;
		onMovedHex();
	}
}


