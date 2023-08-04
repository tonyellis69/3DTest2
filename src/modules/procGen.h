#pragma once

#include <vector>
#include <random>

#include "baseModule.h"

#include "procRoom.h"

class CProcGen : public CBaseModule {
public:
	CProcGen(CHexEngine* engine) : CBaseModule(engine) {}
	void initalise();
	void update(float dt);
	void guiHandler(CGUIevent& e);


private:
	glm::vec3 randomPos();
	glm::i32vec2 randomSize();
	void arrangeRooms();
	bool separateRooms(int maxAttempts);
	bool separateIncrementally(int maxAttempts);
	void resolveOverlap(glm::vec3& overlap, size_t roomA, size_t roomB);
	void resolveOverlapIncr(glm::vec3& overlap, size_t roomA, size_t roomB);
	void cullOutsiders();
	void calcCentreOfMass();
	int convergeRooms(int maxAttempts);
	void moveRoomInward(int roomNo);
	void cullOverlaps();


	CEntity* mainCam;
	CHexArray hexArray;

	std::vector<CProcRoom>  rooms;
	int maxRooms = 30;

	std::mt19937 randEngine;

	glm::vec3 centreOfMass;

	unsigned int seed = 0;
};