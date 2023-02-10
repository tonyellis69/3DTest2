#include "sceneryCollider.h"

#include "entity.h"


std::tuple<bool, glm::vec3> CSceneryColliderCmp::segCollisionCheck(glm::vec3& segA, glm::vec3& segB) {
	if (thisEntity->modelCmp->model.BBcollision(segA, segB))
		return { true, glm::vec3() };

	return { false, glm::vec3() };
}

std::tuple<bool, glm::vec3> CSceneryColliderCmp::entCollisionCheck(CEntity* e2) {
	return { 0, {0,0,0} };
}

std::tuple<float, glm::vec3> CSceneryColliderCmp::sceneryCollisionCheck(CEntity* entity, int dir) {
	glm::vec3 hexPos = cubeToWorldSpace(entity->transform->hexPosition);
	glm::vec3 relativePos = entity->getPos() - hexPos;

	//check for bounding radius intersection with hex boundary segment
	glm::vec3 segA = hexPos + corners[dir];
	glm::vec3 segB = hexPos + corners[(dir + 1) % 6];

	//float radius = body->physics.boundingRadius;
	float radius = entity->collider->boundingRadius;

	glm::vec3 bodyOrigin = entity->getPos();

	glm::vec3 PA = segA - bodyOrigin;
	glm::vec3 segVec = (segB - segA);
	glm::vec3 segVecN = glm::normalize(segVec);

	//project bodyOrigin onto segment, make result a proportion of segment length
	float t = glm::dot(bodyOrigin - segA, segVec) / glm::dot(segVec, segVec);

	//if closest point outside segment, clamp to nearest end
	if (t < 0) t = 0;
	if (t > 1) t = 1;

	glm::vec3 point = segA + t * segVec;

	float dist = glm::distance(bodyOrigin, point);

	if (dist > radius)
		return { 0, {0,0,0} };

	glm::vec3 contactNormal; // = glm::vec3(-segVecN.y, segVecN.x, 0);
	contactNormal = point - bodyOrigin; //this is better for rounding corners
	return { radius - dist, glm::normalize(contactNormal) };
}
