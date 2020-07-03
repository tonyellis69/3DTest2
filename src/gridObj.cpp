#include "gridObj.h"

#include <glm/gtc/matrix_transform.hpp>	


CGridObj::CGridObj() {
	worldPos = glm::vec3(0);
	worldMatrix = &lineModel.model.matrix;
	setZheight(0.05f);

	rotation = 0.0f;
	proximityCutoff = 0.25f;
	prevTargetDist = FLT_MAX;
	moveSpeed = 7.0f;

	buildWorldMatrix();
}

void CGridObj::setHexRenderer(IHexRenderer* rendrObj) {
	hexRendr = rendrObj;
}



/** Set the height at which this object is drawn. */
void CGridObj::setZheight(float height) {
	zHeight = height;
	worldPos.z = height;
}


void CGridObj::setLineModel(CLineModel& lineModel) {
	this->lineModel = lineModel;
	//drawData.lineModel = this->lineModal;
}

void CGridObj::setPosition(int x, int y) {
	worldPos = { x,y,zHeight };
	buildWorldMatrix();
}

void CGridObj::setPosition(CHex& hex) {
	worldPos = cubeToWorldSpace(hex);
	worldPos.z = zHeight;
	buildWorldMatrix();
}

void CGridObj::draw() {
	//hexRendr->drawLines(drawData);
	hexRendr->drawLineModel(lineModel);
}

bool CGridObj::update(float dT) {
	return updateMove(dT);
}

bool CGridObj::updateMove(float dT) {
	glm::vec3 velocity = moveVector * moveSpeed * dT;

	if (glm::distance(worldPos, worldSpaceDestination) < proximityCutoff 
		|| glm::distance(worldPos, worldSpaceDestination) > prevTargetDist) {
		velocity = glm::vec3(0);
		return false;
	}
	else {
		prevTargetDist = glm::distance(worldPos, worldSpaceDestination);
		worldPos += velocity;
	}

	buildWorldMatrix();
	return true;
}

void CGridObj::buildWorldMatrix() {
	*worldMatrix = glm::translate(glm::mat4(1), worldPos);
	*worldMatrix = glm::rotate(*worldMatrix, rotation, glm::vec3(0, 0, -1));

}
