#pragma once

#include <vector>
#include <map>
#include <tuple>

#include "../gamehextObj.h"
#include "hex/hexArray.h"

class CBodyPairKey;
class CBodyPair;


/** Resolves the physics stuff of registered entities. */
class CHexPhysics {
public:
	CHexPhysics();
	void add(CGameHexObj* entity);
	void setMap(CHexArray* hexArray);
	void removeEntities();
	void update(float dT);



private:
	void broadphase();
	void integrateForces();
	void resolveContacts();
	void integrateVelocities();
	std::tuple<float, glm::vec3> findSceneryCollision(CGameHexObj* a, glm::vec3& hexPos, int dir);

	std::vector<CGameHexObj*> entities;
	float dT;

	CHexArray* hexArray;

	CGameHexObj tmpMapObj;

	std::map<CBodyPairKey, CBodyPair> bodyPairs;
};


class CBodyPairKey {
public:
	CBodyPairKey(CGameHexObj* a, CGameHexObj* b, int ref=0) ;
	bool operator < (const CBodyPairKey& a2) const;

	CGameHexObj* objA;
	CGameHexObj* objB;

private:
	int refCount = 0;
};


class CBodyPair {
public:
	CBodyPair() {};
	CBodyPair(CGameHexObj* a, CGameHexObj* b, float p, glm::vec3& n) :
		A(a), B(b), penetration(p), normal(n) {};

	CGameHexObj* A;
	CGameHexObj* B;

	float penetration = 0;
	glm::vec3 normal = { 0,0,0 };



};

