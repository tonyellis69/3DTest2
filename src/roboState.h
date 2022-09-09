#pragma once
#define _USE_MATH_DEFINES

#include <memory>
#include <vector>
#include <glm/glm.hpp>

class CRobot;
class CEntity;
class CRoboState {
public:
	//CRoboState() {};
	CRoboState(CRobot* bot) {
		this->bot = bot;
	}
	virtual std::shared_ptr<CRoboState> update(float dT) = 0;
	virtual glm::vec3 getDestination() {
		return glm::vec3(0);
	}
	virtual void setDestination(const glm::vec3& d) {}


	CRobot* bot;
	float dT;

};

class CRoboWander : public CRoboState {
public:
	CRoboWander(CRobot* bot);
	std::shared_ptr<CRoboState> update(float dT);
	glm::vec3 getDestination();
	void setDestination(const glm::vec3& d) {
		destination = d;
	}

//	bool turnTo(glm::vec3& p);

	glm::vec3 destination = { 0,0,0 };
	float speed = 0.0f;
	float turnDestination = 0; ///<Bearing we want to turn to
	float lastDestinationDist = FLT_MAX;
	float destSlowdownRange = 0.6f;
	float destSlowdownRate = 0.5f;
};

struct TGlance {
	float angle;
	float pause;
};

class CGlanceAround : public CRoboState {
public:
	CGlanceAround(CRobot* bot);
	std::shared_ptr<CRoboState> update(float dT);

	std::vector<TGlance> glances;
	float totalRotation;
	float turnDir;
	float cumulativeRotation;

	float turnSpeed = 5.0f;
	float glanceSpeed = 4.0f;

	float pause = 0;
};

class CCharge : public CRoboState {
public:
	CCharge(CRobot* bot, CEntity* targetEntity);
	std::shared_ptr<CRoboState> update(float dT);
	glm::vec3 getDestination();

	CEntity* targetEntity;
	glm::vec3 destination;
	bool targetInSight = true;
	float meleeRange = 0.75f;
	float chargeSpeed = 3000.0f;
};


class CMelee : public CRoboState {
public:
	CMelee(CRobot* bot, CEntity* targetEntity);
	std::shared_ptr<CRoboState> update(float dT);

	CEntity* targetEntity;
	glm::vec3 lungeVec;
	glm::vec3 startPos;
	float lungeDist;
	float timer;
	float lungeEnd = 0.125f;
	float returnEnd = 0.25f;
	float meleeRange = 0.85f;
};


class CCloseAndShoot : public CRoboState {
public:
	CCloseAndShoot(CRobot* bot, CEntity* targetEntity);
	std::shared_ptr<CRoboState> update(float dT);
	glm::vec3 getDestination();

	CEntity* targetEntity;
	glm::vec3 destination;
	float missileCooldown = 0.0f;
	bool stoppedToShoot = false;
	float idealShootRange = 3;
	float escapeRange = idealShootRange + 1;
	float speed;
	glm::vec3 lastSighting; ///<Of the target.
};

class CGoTo : public CRoboState {
public:
	CGoTo(CRobot* bot, glm::vec3& dest);
	std::shared_ptr<CRoboState> update(float dT);
	glm::vec3 getDestination();

	glm::vec3 destination;
	float speed = 2000.0f;
};



class CDoNothing : public CRoboState {
public:
	CDoNothing(CRobot* bot) : CRoboState(bot) {}
	std::shared_ptr<CRoboState> update(float dT) {
		return nullptr;
	};

};

class CGoToHunting : public CGoTo {
public:
	CGoToHunting(CRobot* bot, glm::vec3& dest, CEntity* quarry);
	std::shared_ptr<CRoboState> update(float dT);

	CEntity* targetEntity;
};