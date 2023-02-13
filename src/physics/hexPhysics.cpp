#include "hexPhysics.h"

#include "utils/log.h"

#include "../entity/sceneryCollider.h"

CHexPhysics::CHexPhysics() {
	tmpMapObj.phys = std::make_shared <CPhys>(&tmpMapObj, 0.0f);
	tmpMapObj.collider = std::make_shared<CSceneryColliderCmp>(&tmpMapObj);

	//TODO: this is really hacky! Probably the map should be an entity or something
}

void CHexPhysics::add(CEntity* entity) {
	entities.push_back(entity);
}

void CHexPhysics::remove(CEntity* removeEntity) {
	for (auto& entity = entities.begin(); entity != entities.end(); entity++) {
		if (*entity == removeEntity) {
			entities.erase(entity);
			return;
		}
	}
}

void CHexPhysics::setMap(CHexArray* hexArray) {
	this->hexArray = hexArray;
}

void CHexPhysics::clearEntities() {
	entities.clear();
}

void CHexPhysics::update(float dT) {
	this->dT = dT;

	broadphase();
	integrateForces();
	resolveContacts(); //happens here
	integrateVelocities();

}

void CHexPhysics::removeDeletedEntities() {
	for (auto& it = entities.begin(); it != entities.end(); ) {
		if ((*it)->deleteMe)
			it = entities.erase(it);
		else
			it++;
	}
}

void CHexPhysics::removeDeadEntities() {
	for (auto& it = entities.begin(); it != entities.end(); ) {
		if ((*it)->live == false)
			it = entities.erase(it);
		else
			it++;
	}
}
//FIXME: scrap above for below

//void CHexPhysics::removeEntities() {
//	for (auto& it = entities.begin(); it != entities.end(); ) {
//		if ((*it)->toRemove == true)
//			it = entities.erase(it);
//		else
//			it++;
//	}
//}

/** Look for possible collisions between scenery and pairs of bodies. */
void CHexPhysics::broadphase() {
	bodyPairs.clear();

	for (unsigned int e1=0; e1<entities.size(); e1++) {
		auto entity = entities[e1];

		if (!entity->collider)
			continue;

		//openHexes.clear();
		std::vector<CHex> openHexes;

		//scenery collisions
		glm::vec3 hexPos = cubeToWorldSpace(entity->transform->hexPosition);
		glm::vec3 relativePos = entity->getPos() - hexPos;
		for (int dir = 0; dir < 6 ; dir++) {
			CHex neighbour = getNeighbour(entity->transform->hexPosition, THexDir(dir));
			if (hexArray->getHexCube(neighbour).content == solidHex) {
				CBodyPairKey key(entity, &tmpMapObj, dir);
				auto [penetration,normal] = findSceneryCollision2(entity,dir);
				if (penetration > 0) {				
					CBodyPair bodyPair(entity, &tmpMapObj, penetration, normal);
					bodyPairs[key] = bodyPair;
				}
				else {
					bodyPairs.erase(key);
				}
			}
			else {
				openHexes.push_back(neighbour);
			}

		}

		if (entity->collider->sceneryOnly)
			continue;


		//entity-entity collisions
		for (unsigned int e2 = e1 + 1; e2 < entities.size(); e2++) {
			auto entity2 = entities[e2];
			if (!entity2->collider || entity2->collider->sceneryOnly)
				continue;

			if (entity->transform->hexPosition == entity2->transform->hexPosition ||
				std::count(openHexes.begin(), openHexes.end(), entity2->transform->hexPosition)) {
				CBodyPairKey key(entity, entity2);
				auto [penetration2, normal2] = findEntityCollision(entity, entity2);
				if (penetration2 > 0) {
					CBodyPair bodyPair(entity, entity2, penetration2, normal2);
					bodyPairs[key] = bodyPair;
				}
				else {
					bodyPairs.erase(key);
				}
			}
		}

	}

}

/** Find velocities of all entities. */
void CHexPhysics::integrateForces() {
	for (auto& entity : entities) {
		CPhys* ent = entity->phys.get();
		glm::vec3 a = (ent->invMass * ent->moveImpulse) * dT;
		ent->velocity += a;
		ent->moveImpulse = { 0, 0, 0};

		//drag
		ent->velocity *= std::pow(ent->drag, dT);
		//bigger time-gap = smaller fraction by which we scale velocity
		//so over time velocity will never rise above a certain value for a given acceleration
		//The closer drag is to 0, the faster we slow down


	}

}

/** Ensure all contacting bodies are separating. */
void CHexPhysics::resolveContacts() {
	for (int i = 0; i < 10; i++) {
		for (auto& [key,bodyPair] : bodyPairs) {
			CPhys* physA = bodyPair.A->phys.get();
			CPhys* physB = bodyPair.B->phys.get();

			glm::vec3 relativeVelocity = physB->velocity - physA->velocity;
			float velocityAlongNormal = glm::dot(relativeVelocity, bodyPair.normal);

			//Separating? Then don't resolve
			if (velocityAlongNormal > 0) 
				continue;

			//find the restitution ('bounce')
			float e = std::min(physA->restitution, physB->restitution);

			//calcualate the impulse scalar
			float j = -(1 + e) * velocityAlongNormal;
			j /= physA->invMass + physB->invMass;

			//share the separating impulse between the bodies by mass
			glm::vec3 impulse = j * bodyPair.normal;
			physA->velocity -= physA->invMass * impulse;
			physB->velocity += physB->invMass * impulse;

		}


	}


}

/** Find new position of all entities from velocity. */
void CHexPhysics::integrateVelocities() {
	for (auto& entity : entities) {
		entity->transform->updatePos(entity->phys->velocity * dT);

	}
}

std::tuple<float, glm::vec3> CHexPhysics::findSceneryCollision(CEntity* body, glm::vec3& hexPos, int dir) {

	//check for bounding radius intersection with hex boundary segment
	glm::vec3 segA = hexPos + corners[dir];
	glm::vec3 segB = hexPos + corners[(dir + 1) % 6];

	//float radius = body->physics.boundingRadius;
	float radius = body->collider->boundingRadius;

	glm::vec3 bodyOrigin = body->getPos();

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
	contactNormal = point -  bodyOrigin; //this is better for rounding corners
	return { radius - dist, glm::normalize(contactNormal) };

}

std::tuple<float, glm::vec3> CHexPhysics::findEntityCollision(CEntity* entA, CEntity* entB) {
	if (entA->collider->colliderType == missileCollider)
		return entA->collider->entCollisionCheck(entB);
	if (entB->collider->colliderType == missileCollider)
		return entB->collider->entCollisionCheck(entA);
	//FIXME: naive! There will be other combinations




	if (entA->collider->colliderType == rigidBodyCollider
		&& entB->collider->colliderType == rigidBodyCollider) {
		return entA->collider->entCollisionCheck(entB);
	}
	return { 0, {0,0,0} };
}

std::tuple<float, glm::vec3> CHexPhysics::findSceneryCollision2(CEntity* entity,  int dir) {
	if (entity->collider->colliderType == missileCollider)
		return { 0, {0,0,0} }; //no physics collision to resolve, leave to collider.

	//checks for collider types go here
	return tmpMapObj.collider->sceneryCollisionCheck(entity, dir);


	return { 0, {0,0,0} };
}


CBodyPairKey::CBodyPairKey(CEntity* a, CEntity* b, int ref ) {
	if (a < b) {
		objA = a;
		objB = b;
	}
	else {
		objA = b;
		objB = a;
	}

	refCount = ref;
}

bool CBodyPairKey::operator<(const CBodyPairKey& a2) const {
	if (objA < a2.objA)
		return true;

	if (objA == a2.objA && objB < a2.objB)
		return true;

	if (objA == a2.objA && objB == a2.objB && refCount < a2.refCount)
		return true;

	return false;
}
