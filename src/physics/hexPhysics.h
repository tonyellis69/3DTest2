#pragma once

#include <vector>
#include <map>
#include <tuple>

//#include "../gamehextObj.h"
#include "../entity/entity.h"
#include "hex/hexArray.h"

#include "../gameEvent.h"
//TODO: supply event type rather than use gameEvent, for true decoupling
//or if we're just calling one function, simply make a callback via lambda

#include "../physEvent.h"

class CBodyPairKey;
class CBodyPair;


/** Resolves the physics stuff of registered entities. */
class CHexPhysics {
public:
	CHexPhysics();
	void onEvent(CGameEvent& e);
	void onEvent(CPhysicsEvent& e);
	void add(CEntity* entity);
	void remove(CEntity* entity);
	void setMap(CHexArray* hexArray);
	void clearEntities();
	void update(float dT);




private:
	void broadphase();
	void integrateForces();
	void resolveContacts();
	void integrateVelocities();
	std::tuple<float, glm::vec3> findSceneryCollision(CEntity* a, glm::vec3& hexPos, int dir);
	std::tuple<float, glm::vec3> findEntityCollision(CEntity* entA, CEntity* entB);
	std::tuple<float, glm::vec3> findSceneryCollision2(CEntity* entity, int dir);

	std::vector<CEntity*> entities;
	float dT;

	CHexArray* hexArray;

	CEntity tmpMapObj;

	std::map<CBodyPairKey, CBodyPair> bodyPairs;
};


class CBodyPairKey {
public:
	CBodyPairKey(CEntity* a, CEntity* b, int ref=0) ;
	bool operator < (const CBodyPairKey& a2) const;

	CEntity* objA;
	CEntity* objB;

private:
	int refCount = 0;
};


class CBodyPair {
public:
	CBodyPair() {};
	CBodyPair(CEntity* a, CEntity* b, float p, glm::vec3& n) :
		A(a), B(b), penetration(p), normal(n) {};

	CEntity* A;
	CEntity* B;

	float penetration = 0;
	glm::vec3 normal = { 0,0,0 };



};

