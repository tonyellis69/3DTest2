#include "transform.h"

#include "entity.h"

void CTransformCmp::setRotation(float angle) {
	parentEntity->setRotation(angle);
}

void CTransformCmp::setPos(glm::vec3& pos) {
	parentEntity->setPosition(pos);
}

void CTransformCmp::setPosOffGrid(glm::vec3& pos) {
	parentEntity->setPosition(pos);
	parentEntity->hexPosition = CHex(-1);
}

void CTransformCmp::setScale(glm::vec3& scale) {
	parentEntity->setScale(scale);
}

void CTransformCmp::buildWorldMatrix() {
	parentEntity->buildWorldMatrix();
}

void CTransformCmp::update(float dT) {
	buildWorldMatrix();
}
