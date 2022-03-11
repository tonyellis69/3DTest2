#pragma once
#define _USE_MATH_DEFINES

#include "entity.h"
#include "roboState.h"

enum TRobotState { robotLightSleep,	robotWander3, robotCharge3,
	robotMelee3, robotCloseAndShoot, robotDoNothing
};

enum TrackingState {trackNone, trackEntity, trackPos, trackEnding};

enum TLungeState { preLunge, lunging, returning };

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CEntity {
public:
	CRobot();
	void setModel(CModel& model);
	void update(float dT);
	void setState(TRobotState newState, CEntity* entity = NULL);
	void setRotation(float angle);
	void rotate(float angle);
	void setUpperRotation(float angle);
	void rotateUpper(float angle);
	void draw();
	std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB);
	void buildWorldMatrix();
	void startTracking(CEntity* target);
	void startTracking(glm::vec3& pos);
	void stopTracking();


	void receiveDamage(CEntity& attacker, int damage);
	void setImpulse(glm::vec3& dest, float maxSpeed);
	glm::vec3 slowTo(glm::vec3& pos);
	bool clearLineTo(const glm::vec3& p);
	bool canSeePlayer();
	bool clearLineTo(CEntity* target);
	void fireMissile(CEntity* target);
	bool turnTo(glm::vec3& p);
	void stopMoving();
	glm::vec3* getDestination();
	glm::vec3 findAvoidance();
	glm::vec3 findAvoidance2();
	void headTo(glm::vec3& pos);

	float upperBodyRotation = 0;
	bool upperBodyLocked = true; ///<If true, rotate upper body with base.

	float avoidanceDist = 2.0f; ///<Arbitrary check-ahead distance.

	glm::vec3 lAvoidVec[2];
	glm::vec3 rAvoidVec[2];
	bool lObstacle;
	bool rObstacle;

	float chosenSpeed;
	const float defaultSpeed = 1000;
	const float maxSpeed = 3000;

private:
	bool inFov(CEntity* target);
	void onMovedHex();

	void trackTarget();

	void updateTreadCycle();

	float dT;
	int hp = 3; 

	std::shared_ptr<CRoboState> currentState = nullptr; ///<The robot's current behaviour.

	float slowingDist = 0.6f;
	float lastTurnDir = 0;
	float upperTurnSpeed = 5.0f;
	
	TModelMesh* upperBody;
	TModelMesh* base;
	TModelMesh* treads;

	TrackingState trackingState = trackNone; ///<What, if anything, we're keeping upper body pointing at.
	CEntity* trackingEntity = nullptr;
	glm::vec3 trackingPos;

	float treadCycle = 0; ///<Where we are in the tread animation.
	bool moving = false; ///<True if we're motoring somewhere.
	float treadTranslate = 0; ///<Movement for tread animation.


};


