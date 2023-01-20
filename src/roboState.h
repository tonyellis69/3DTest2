#pragma once
#define _USE_MATH_DEFINES

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "entity/aiCmp.h"

enum TrackingState { trackNone, trackEntity, trackPos, trackEnding };

class CRobot;
class CEntity;

struct TObstacle { glm::vec3 pos = glm::vec3(0); float radius; CRobot* bot = nullptr; };

class CRoboState : public CAiCmp {
public:
	CRoboState(CRobot* bot)  : CAiCmp((CEntity*)bot) {
		this->pBot =bot;
	}
	void update(float dT);
	void trackTarget();
	void startTracking(CEntity* target);
	void startTracking(glm::vec3& pos);
	void stopTracking();
	void updateTreadCycle();
	glm::vec3 arriveAt(glm::vec3& dest);
	float speedFor(glm::vec3& dest);
	void stopMoving();
	void headTo(glm::vec3& destinationPos);
	std::tuple<float, float> findAvoidance();
	std::vector<TObstacle> findNearObstacles(glm::vec3& centre);
	std::tuple<TObstacle, glm::vec3> findCollidable(std::vector<TObstacle>& obstacles, glm::vec3& aheadSegBegin, glm::vec3& aheadSegEnd);
	void amIStuck();
	void abortDestination();
	bool turnToward(glm::vec3& dir);
	void turnUpperBodyTo(float destAngle);
	bool inFov(CEntity* target);
	bool clearLineTo(CEntity* target);
	bool clearLineTo(const glm::vec3& p);
	bool canSeeEnemy();
	virtual std::shared_ptr<CRoboState> updateState(float dT) = 0;
	virtual glm::vec3 getDestination() {
		return destination;
	}
	virtual void setDestination(const glm::vec3& d) {
		destination = d;
	}


	glm::vec3 destination = { 0,0,0 };

	CRobot* pBot; //helpfully points to parent entity as a robot.
	float dT;

	TrackingState trackingState = trackNone; ///<What, if anything, we're keeping upper body pointing at.
	CEntity* trackingEntity = nullptr;
	glm::vec3 trackingPos;

	float treadCycle = 0; ///<Where we are in the tread animation.
	bool moving = false; ///<True if we're motoring somewhere.
	float treadTranslate = 0; ///<Movement for tread animation.

	float slowingDist = 0.6f;
	float lastTurnDir = 0;
	float upperTurnSpeed = 0.1f;

	float chosenSpeed;
	const float defaultSpeed = 1000;// 1000;
	const float maxSpeed = 1000; //3000
	float maxTurnSpeed = 3.0f;

	float stuckCheck = 0; ///<Seconds since last check
	float destinationDist = FLT_MAX; ///<Distance to destination on last check.

	CRobot* pRoboCollidee;
	float backingUp = 0;
	float maxAvoidanceDist = 2.5f; ///<Arbitrary check-ahead distance.


	glm::vec3 lAvoidVec[2];
	glm::vec3 rAvoidVec[2];
	bool lObstacle;
	bool rObstacle;
	glm::vec3 tmpCollisionPt;
	glm::vec3 tmpCollisionSegPt;
	glm::vec3 tmpAheadVecEnd;;
	glm::vec3 tmpAheadVecBegin;;

	float robotRadius = 0.7f;
	float ignorable = 0.08f; //below .1 to avoid snapping to some directions, above .03 to avoid quiver
	float obstacleProximityLimit = 0.75f; //0.7f
	float obstacleAhead = 0.9f;
	float obstacleToSide = 0.2f;

	float safeDistAdjust = 0.0f;


};

class CRoboWander : public CRoboState {
public:
	CRoboWander(CRobot* bot);
	//void update(float dT);
	std::shared_ptr<CRoboState> updateState(float dT);


//	bool turnToward(glm::vec3& p);

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
	std::shared_ptr<CRoboState> updateState(float dT);

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
	std::shared_ptr<CRoboState> updateState(float dT);


	CEntity* targetEntity;

	bool targetInSight = true;
	float meleeRange = 1.7f; // 0.95f;// 0.75f;
	float chargeSpeed = 1750.0f;
};


class CMelee : public CRoboState {
public:
	CMelee(CRobot* bot, CEntity* targetEntity);
	std::shared_ptr<CRoboState> updateState(float dT);

	CEntity* targetEntity;
	glm::vec3 lungeVec;
	glm::vec3 startPos;
	float lungeDist;
	float timer;
	float lungeEnd = 0.125f;
	float returnEnd = 0.25f;
	float meleeRange = 1.75f;
	bool hit = false;
};


class CCloseAndShoot : public CRoboState {
public:
	CCloseAndShoot(CRobot* bot, CEntity* targetEntity);
	std::shared_ptr<CRoboState> updateState(float dT);
	//glm::vec3 getDestination();

	CEntity* targetEntity;
	//glm::vec3 destination;
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
	std::shared_ptr<CRoboState> updateState(float dT);

	float speed = 2000.0f;
};



class CDoNothing : public CRoboState {
public:
	CDoNothing(CRobot* bot) : CRoboState(bot) {}
	std::shared_ptr<CRoboState> updateState(float dT) {
		return nullptr;
	};

};

class CGoToHunting : public CGoTo {
public:
	CGoToHunting(CRobot* bot, glm::vec3& dest, CEntity* quarry);
	std::shared_ptr<CRoboState> updateState(float dT);

	CEntity* targetEntity;
};


class CTurnToSee : public CRoboState {
public:
	CTurnToSee(CRobot* bot, glm::vec3& dest);
	std::shared_ptr<CRoboState> updateState(float dT);

	glm::vec3 dir;
};