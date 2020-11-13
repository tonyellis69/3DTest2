#include "sprite.h"

#include <glm/gtc/matrix_transform.hpp>	

CSprite::CSprite() {
	worldPos = glm::vec3(0);
	worldMatrix = &lineModel.model.matrix;
	rotation = 0;
}

void CSprite::update(float dT) {
	
}

void CSprite::draw() {

}

void CSprite::setPosition(glm::vec3& pos) {
	worldPos = pos;
	buildWorldMatrix();
}

void CSprite::setPosition(glm::vec3& pos, float rotation) {
	worldPos = pos;
	this->rotation = rotation;
	buildWorldMatrix();
}

void CSprite::buildWorldMatrix() {
	*worldMatrix = glm::translate(glm::mat4(1), worldPos);
	*worldMatrix = glm::rotate(*worldMatrix, rotation, glm::vec3(0, 0, 1));
}
