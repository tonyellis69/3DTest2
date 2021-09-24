#pragma once
#define _USE_MATH_DEFINES

#include <memory>
#include <vector>
#include <glm/glm.hpp>

class CRobot;
class CRoboState {
public:
	//CRoboState() {};
	CRoboState(CRobot* bot) {
		this->bot = bot;
	}
	virtual std::shared_ptr<CRoboState> update(float dT) = 0;
	CRobot* bot;
	float dT;

};

class CRoboWander : public CRoboState {
public:
	CRoboWander(CRobot* bot);
	std::shared_ptr<CRoboState> update(float dT);

	bool turnTo(glm::vec3& p);

	glm::vec3 destination = { 0,0,0 };
	float speed = 0.0f;
	float turnDestination = 0; ///<Bearing we want to turn to
	float lastDestinationDist = FLT_MAX;
	float destSlowdownRange = 0.4f;
	float destSlowdownRate = 0.5f;

	float lastTurnDir = 0;
};

class CGlanceAround : public CRoboState {
public:
	CGlanceAround(CRobot* bot);
	std::shared_ptr<CRoboState> update(float dT);

	std::vector<float> glances;
	float totalRotation;
	float turnDir;
	float cumulativeRotation;

	float turnSpeed = 5.0f;
	float glanceSpeed = 3.0f;

	float pause = 0;
};