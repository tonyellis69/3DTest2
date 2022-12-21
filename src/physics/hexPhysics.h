#pragma once

#include <vector>
#include <map>
#include <tuple>

//#include "../gamehextObj.h"
#include "../entity/entity.h"
#include "hex/hexArray.h"

class CBodyPairKey;
class CBodyPair;


/** Resolves the physics stuff of registered entities. */
class CHexPhysics {
public:
	CHexPhysics();
	void add(CEntity* entity);
	void setMap(CHexArray* hexArray);
	void clearEntities();
	void update(float dT);
	void removeDeletedEntities();
	void removeDeadEntities();
	//void removeEntities();

private:
	void broadphase();
	void integrateForces();
	void resolveContacts();
	void integrateVelocities();
	std::tuple<float, glm::vec3> findSceneryCollision(CEntity* a, glm::vec3& hexPos, int dir);

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

