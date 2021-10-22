#include "hexPhysics.h"

#include "utils/log.h"

CHexPhysics::CHexPhysics() {
	tmpMapObj.physics.invMass = 0;// FLT_MAX;
}

void CHexPhysics::add(CEntity* entity) {
	entities.push_back(entity);
}

void CHexPhysics::setMap(CHexArray* hexArray) {
	this->hexArray = hexArray;
}

void CHexPhysics::removeEntities() {
	entities.clear();
}

void CHexPhysics::update(float dT) {
	this->dT = dT;

	broadphase();
	integrateForces();
	resolveContacts();
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

/** Look for possible collisions between scenery and pairs of bodies. */
void CHexPhysics::broadphase() {
	bodyPairs.clear();
	//std::vector<CHex> openHexes;
	//openHexes.reserve(6);

	for (unsigned int e1=0; e1<entities.size(); e1++) {
		auto entity = entities[e1];
		//openHexes.clear();
		std::vector<CHex> openHexes;

		//scenery collisions
		glm::vec3 hexPos = cubeToWorldSpace(entity->hexPosition);
		glm::vec3 relativePos = entity->worldPos - hexPos;
		for (int dir = 0; dir < 6 ; dir++) {
			CHex neighbour = getNeighbour(entity->hexPosition, THexDir(dir));
			if (hexArray->getHexCube(neighbour).content == solidHex) {
				CBodyPairKey key(entity, &tmpMapObj, dir);
				auto [penetration,normal] = findSceneryCollision(entity,hexPos,dir);
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

		//entity-entity collisions
		for (unsigned int e2 = e1 + 1; e2 < entities.size(); e2++) {
			auto entity2 = entities[e2];
			if (entity->hexPosition == entity2->hexPosition ||
				std::count(openHexes.begin(), openHexes.end(), entity2->hexPosition)) {
				CBodyPairKey key(entity, entity2);
				auto [penetration2, normal2] = entity->collisionCheck(entity2);
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
		CPhys* ent = &entity->physics;
		glm::vec3 a = ent->invMass * ent->moveImpulse;
		ent->velocity += a * dT;
		ent->moveImpulse = { 0, 0, 0};

		//drag
		//ent->velocity = glm::mix(ent->velocity, glm::vec3(0), 1.0f - std::pow(ent->drag, dT)); //larger dt, closer v gets to 0
		ent->velocity *= std::pow(ent->drag, dT);
		//larger number, slower falloff

		//if (entity->tmpId == 0)
		//	sysLog << "\nphysics inc to velocity: " << a * dT << " final velocity " << ent->velocity;

	}

}

/** Ensure all contacting bodies are separating. */
void CHexPhysics::resolveContacts() {
	for (int i = 0; i < 10; i++) {
		for (auto& [key,bodyPair] : bodyPairs) {
			CPhys* physA = &bodyPair.A->physics;
			CPhys* physB = &bodyPair.B->physics;

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
		entity->updatePos(entity->physics.velocity * dT);

	}
}

std::tuple<float, glm::vec3> CHexPhysics::findSceneryCollision(CEntity* body, glm::vec3& hexPos, int dir) {

	//check for bounding radius intersection with hex boundary segment
	glm::vec3 segA = hexPos + corners[dir];
	glm::vec3 segB = hexPos + corners[(dir + 1) % 6];

	float radius = body->physics.boundingRadius;
	glm::vec3 bodyOrigin = body->worldPos;

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
