#pragma once

#include <vector>
#include <random>

#include "baseModule.h"

#include "procRoom.h"
#include "doorRect.h"
#include "doorBot.h"
#include "indiRect.h"

class CProcGen : public CBaseModule {
public:
	CProcGen(CHexEngine* engine) : CBaseModule(engine) {}
	void initalise();
	void update(float dt);
	void guiHandler(CGUIevent& e);

	struct edge {
		int a;
		int b;
		bool operator==(const edge& rhs) {
			return  (a == rhs.a && b == rhs.b) || (b == rhs.a && a == rhs.b);
		}
	};

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
	void triangulate();
	void createMST();
	void createDoorways();
	void createIndiRect(CDoorRect& failDoorRect);
	void drawHexes();
	void initPaths();
	void runDoorBots();
	void drawPaths();
	void fillRooms();
	void findPath(int roomA, int roomB);
	void findPathDijkstra(int roomA, int roomB);
	void solveDoorPaths();


	CEntity* mainCam;
	CHexArray hexArray;

	std::vector<CProcRoom>  rooms;
	int maxRooms = 5;// 30;

	std::mt19937 randEngine;

	glm::vec3 centreOfMass;

	unsigned int seed = 76;// 180;// 76;// 164;// 76;//has a too-small overlap


	std::vector<edge> triEdges;
	std::vector<edge> mstEdges;

	bool showHexes = false;

	std::vector<CDoorRect>  doorRects;
	std::vector<CDoorBot>  doorBots;
	std::vector<CIndiRect>  indiRects;

	std::unordered_map<glm::i32vec2, CHex> cameFrom;
};

