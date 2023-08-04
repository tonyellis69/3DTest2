#include "procGen.h"

#include <glm/glm.hpp>

#include <algorithm>

#include "../gameState.h"

#include "../entity/cameraC.h"

#include "win/win.h"

#include "utils/random.h"

#include "renderer/imRendr/imRendr.h"


void CProcGen::initalise() {

	randEngine.seed(555555555);

	mainCam = gameWorld.spawn("mainCam");
	mainCam->getComponent<CameraC>()->setZoom2Fit(true);

	hexArray.initialise(60, 40);

	gameWorld.updateHexMap(hexArray);


	CWin::showMouse(true);
	CWin::fullScreen();
}

void CProcGen::update(float dt) {
	for (auto& room : rooms) {
		room.drawWireFrame();
	}

	imRendr::drawLine(centreOfMass - glm::vec3(3,0,0), centreOfMass + glm::vec3(3, 0, 0));
	imRendr::drawLine(centreOfMass - glm::vec3(0, 3, 0), centreOfMass + glm::vec3(0, 3, 0));

}

void CProcGen::guiHandler(CGUIevent& e)
{
	if (e.type == eKeyDown) {
		if (e.key == 'L') {
			separateRooms(1);
			cullOutsiders();
		}
		if (e.key == 'C') {
			convergeRooms(3);
		}
		if (e.key == 'P') {
			separateIncrementally(1);
		}
		if (e.key == 'K') {
			cullOverlaps();
		}
		if (e.key == 'O') {
			arrangeRooms();
		}
		if (e.key == 'H') {
			for (auto& room : rooms) {
				room.writeHexes(hexArray);
			}
			gameWorld.updateHexMap(hexArray);
		}

	}



}

glm::vec3 CProcGen::randomPos() {
	static std::normal_distribution<float> p{0, 5};
	return glm::vec3(p(randEngine), p(randEngine), 0);
}

glm::i32vec2 CProcGen::randomSize() {
	static std::normal_distribution<float> s{8, 2};

	int w = std::max<int>(int(s(randEngine)), 4);
	int h = std::max<int>(int(s(randEngine)), 4);

	if (h & 1) //height must always be even to centre verts
		h++;

	//fiddle-faddle to ensure origin as well as corners fall on a precise hex.
	if ((h / 2) & 1 ) { //if half height odd
		if ((w & 1) == 0)
			w++;
	}
	else { //if half height even
		if (w & 1)
			w++;
	}
	
	return glm::i32vec2(w,h);
}

/** Steer rooms into their final positions, culling as necessary. */
void CProcGen::arrangeRooms() {
	randEngine.seed(seed++);
	rooms.clear();
	for (int r = 0; r < maxRooms; r++) {
		rooms.push_back(CProcRoom(randomPos(), randomSize()));
	}

	bool separated = separateRooms(20);

	if (separated == false) {
		separated = separateIncrementally(8);
	}

	if (separated == false) {
		cullOverlaps();
	}
}

bool CProcGen::separateRooms(int maxAttempts) {
	bool colliding = true;
	int attemps = 0;
	while (colliding) {
		colliding = false;
		calcCentreOfMass();
		for (size_t i = 0; i < rooms.size(); i++) {
			for (size_t j = i + 1; j < rooms.size(); j++) {
				glm::vec3 overlap = rooms[i].findOverlap(rooms[j]);
				if (glm::length(overlap) > 0.0f) {
					resolveOverlap(overlap, i, j);
					colliding = true;
				}
			}
		}
		cullOutsiders();
		attemps++;
		if (attemps == maxAttempts)
			return false;
	}

	return true;
}

bool CProcGen::separateIncrementally(int maxAttempts) {
	bool colliding = true;
	int attemps = 0;
	calcCentreOfMass();
	while (colliding)	{
		colliding = false;
		for (size_t i = 0; i < rooms.size(); i++) {
			for (size_t j = i + 1; j < rooms.size(); j++) {
				glm::vec3 overlap = rooms[i].findOverlap(rooms[j]);
				if (glm::length(overlap) > 0.0f) {
					resolveOverlapIncr(overlap, i, j);
					colliding = true;
				}
			}
		}
		cullOutsiders();
		attemps++;
		if (attemps == maxAttempts)
			return false;
	}
	return true;
}

void CProcGen::resolveOverlap(glm::vec3& overlap, size_t roomA, size_t roomB) {
	if (glm::distance(rooms[roomB].getOrigin(), centreOfMass) > glm::distance(rooms[roomA].getOrigin(), centreOfMass)) {
		std::swap(roomA, roomB);
		overlap *= -1.0f;
	}

	glm::i32vec2 hexOverlap; glm::i32vec2 nudge(0);
	if (abs(overlap.x) < abs(overlap.y)) {		
		hexOverlap = { overlap.x / (hexWidth - 0.0001f),0};
	}
	else {
		hexOverlap = {0,overlap.y / hexRowHeight };
		hexOverlap *= -1;
	}

	glm::i32vec2 moveA(0); glm::i32vec2 moveB(0);
	moveA = (hexOverlap / 2) + (hexOverlap % 2);
	moveB = hexOverlap - moveA;

	rooms[roomA].move(-moveA + nudge);
	rooms[roomB].move(moveB);
}

void CProcGen::resolveOverlapIncr(glm::vec3& overlap, size_t roomA, size_t roomB) {
	if (glm::distance(rooms[roomB].getOrigin(), centreOfMass) > glm::distance(rooms[roomA].getOrigin(), centreOfMass)) {
		std::swap(roomA, roomB); //ensures furthest-out rect moves, prevents pile-ups 
		overlap *= -1.0f;
	}

	glm::i32vec2 moveStep;
	bool xMove;
	if (abs(overlap.x) < abs(overlap.y)) {
		moveStep = { overlap.x > 0 ? -1 : 1 , 0 };
		xMove = true;
		if (fabs(overlap.x - halfHexWidth) < 0.0001f || fabs(overlap.x + halfHexWidth) < 0.0001f) {//half-hexWidth overlap		 
			moveStep = { 0,  overlap.y > 0 ? -1 : 1 };  //nudge in direction of overlap to ensure more shared wall
		}
	}
	else {
		xMove = false;
		moveStep = { 0, overlap.y > 0 ? 1 : -1 };
	}

	rooms[roomA].move(moveStep );
}

void CProcGen::cullOutsiders() {
	glm::vec3 halfGridSize = glm::vec3( hexArray.width * hexWidth, hexArray.height * hexRowHeight, 0 ) / 2.0f;

	for (auto it = rooms.begin(); it != rooms.end();) {
		if (it->notInside(halfGridSize))
			it = rooms.erase(it);
		else
			it++;
	}
}

void CProcGen::calcCentreOfMass() {
	centreOfMass = glm::vec3(0);
	for (auto& room : rooms) {
		centreOfMass += room.getOrigin();
	}
	centreOfMass /= rooms.size();
}

/** Move isolated rooms toward the centre. */
int CProcGen::convergeRooms(int maxAttempts) {
	bool isolatedRooms = true;
	int attempts = 0;
	while (isolatedRooms) {
		isolatedRooms = false;
		for (size_t i = 0; i < rooms.size(); i++) {
			bool isolated = true;
			for (size_t j = 0; j < rooms.size(); j++) {
				if (i != j && rooms[i].isApartFrom(rooms[j]) == false)
					isolated = false;
			}
			if (isolated) {
				moveRoomInward(i);
				isolatedRooms = true;
			}
		}
		if (isolatedRooms)
			attempts++;
		if (attempts == maxAttempts)
			break;

	}
	return attempts;
}

void CProcGen::moveRoomInward(int roomNo) {
	glm::vec3 dirToCentre = centreOfMass - rooms[roomNo].getOrigin();

	glm::i32vec2 moveStep;
	if (fabs(dirToCentre.x) > fabs(dirToCentre.y)) {
		moveStep = { dirToCentre.x > 0 ? 1 : -1 , 0 };
	}
	else {
		moveStep = {0,  dirToCentre.y > 0 ? -1 : 1  };
	}
	rooms[roomNo].move(moveStep);
}

void CProcGen::cullOverlaps() {
	bool overlaps = true;
	while (overlaps) {
		overlaps = false;
		int maxOverlaps = 0;
		int mostOverlapped = -1;
		for (size_t i = 0; i < rooms.size(); i++) {
			int nOverlaps = 0;
			for (size_t j = 0; j < rooms.size(); j++) {
				if (i != j && glm::length(rooms[i].findOverlap(rooms[j])) > 0.0f) {
					nOverlaps++;
					overlaps = true;
				}
			}
			if (nOverlaps > maxOverlaps) {
				maxOverlaps = nOverlaps;
				mostOverlapped = i;
			}
		}

		if (mostOverlapped > -1) 
			rooms.erase(rooms.begin() + mostOverlapped);
	}
}
