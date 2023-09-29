#include "procGen.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <set>
#include <queue>

#include "../gameState.h"

#include "../entity/cameraC.h"

#include "win/win.h"

#include "utils/random.h"

#include "renderer/imRendr/imRendr.h"

#include "delaunator.hpp"

#include "hex/hex.h"



void CProcGen::initalise() {

	randEngine.seed(555555555);

	mainCam = gameWorld.spawn("mainCam");
	mainCam->getComponent<CameraC>()->setZoom2Fit(true);

	hexArray.initialise(80,60); // (60, 40);

	gameWorld.updateHexMap(hexArray);


	CWin::showMouse(true);
	CWin::fullScreen();
}

void CProcGen::update(float dt) {
	for (auto& room : rooms) {
		room.drawWireFrame();
	}

	//imRendr::drawLine(centreOfMass - glm::vec3(3,0,0), centreOfMass + glm::vec3(3, 0, 0));
	//imRendr::drawLine(centreOfMass - glm::vec3(0, 3, 0), centreOfMass + glm::vec3(0, 3, 0));


	//draw tris
	if (!triEdges.empty()) {
		for (auto& triEdge : triEdges) {
			imRendr::drawLine(rooms[triEdge.a].getOrigin(), rooms[triEdge.b].getOrigin());
		}
	}

	if (!mstEdges.empty()) {
		triEdges.clear();
		for (auto& mstEdge : mstEdges) {
			imRendr::drawLine(rooms[mstEdge.a].getOrigin(), rooms[mstEdge.b].getOrigin());
		}
	}

	if (!doorRects.empty()) {
		for (auto& rect : doorRects) {
			rect.drawWireFrame();
		}
	}

	for (auto& bot : doorBots) {
		bot.drawPos();
	}

	drawPaths();


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
		if (e.key == 'T') {
			triangulate();
		}
		if (e.key == 'M') {
			createMST();
		}
		if (e.key == 'R') {
			createDoorways();
			initPaths();
			runDoorBots();
		}
		if (e.key == 'H') {
			showHexes = !showHexes;
			if (showHexes) {
				drawHexes();
				fillRooms();
			}
			else {
				hexArray.clear();
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
	static std::normal_distribution<float> s{12, 3}; // s{ 8, 2 };

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
	triEdges.clear();
	mstEdges.clear();
	doorRects.clear();
	doorBots.clear();
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

/**	Find the indices that triagulate the room centres. */
void CProcGen::triangulate() {
	//convert origins to Delaunator format
	std::vector<double> coords;
	for (auto& room : rooms) {
		glm::vec3 origin = room.getOrigin();
		coords.emplace_back(double(origin.x));
		coords.emplace_back(double(origin.y));
	}

	//triangulate
	delaunator::Delaunator d(coords);

	//fill with triangle edge indices
	triEdges.clear();
	for (int tri = 0; tri < d.triangles.size(); tri += 3) {
		int a = d.triangles[tri];
		int b = d.triangles[tri + 1];
		int c = d.triangles[tri + 2];

		triEdges.push_back({ a,b });
		triEdges.push_back({ b,c });
		triEdges.push_back({ c,a });
	}
}

void CProcGen::createMST() {
	mstEdges.clear();
	std::set<int> inMST;

	inMST.insert(0); //makes this the starting vertex

	while (inMST.size() != rooms.size() ) {
		float smallestWeight = FLT_MAX;
		edge minEdge;
		int newVert;
		for (int v = 0; v < rooms.size(); v++) {
			if (inMST.count(v) == false) {
				for (int v1 = 0; v1 < rooms.size(); v1++) {
					if (inMST.count(v1) ) {
						for (auto& triEdge : triEdges) {
							if (triEdge == edge{v, v1}) {
								float dist = glm::distance(rooms[triEdge.a].getOrigin(), rooms[triEdge.b].getOrigin());
								if (dist < smallestWeight) {
									smallestWeight = dist;
									minEdge = triEdge;
									newVert = v;
								}
							}
						}
					}
				}
			}
		}
		if (smallestWeight < FLT_MAX) {
			mstEdges.push_back(minEdge);
			inMST.insert(newVert);
		}

	}

}

/** Create rects for each doorway, establishing orientation, size. */
void CProcGen::createDoorways() {
	doorRects.clear();
	for (auto& edge : mstEdges) {
		doorRects.push_back(CDoorRect(rooms[edge.a], rooms[edge.b]));
	}
}

/** Write rooms as hexes to our hexArray. */
void CProcGen::drawHexes() {
	for (auto& room : rooms) {
		room.writeHexes(hexArray);
	}

	//draw doorways
	//for (auto& doorRect : doorRects) {
	//	doorRect.writeHexes(hexArray);
	//}




}

void CProcGen::initPaths() {
	//for (int x = 0; x < hexArray.width; x++)
	//	for (int y = 0; y < hexArray.height; y++) {
	//		 hexArray.getHexOffset(x, y).cost = 100;
	//}
}

/** Advance doorbots by one tick. */
void CProcGen::runDoorBots() {
	if (doorBots.empty()) {
		for (auto& edge : mstEdges) {
			doorBots.push_back(CDoorBot(hexArray, rooms[edge.a].getOriginHex(),rooms[edge.b].getOriginHex()));
		}
		return;
	}

	for (auto& bot : doorBots) {
		bot.update();


	}
 
}

void CProcGen::drawPaths() {
	float size = 0.5f;
	imRendr::setDrawColour({ 1,1,1,0.75f });

	for (int x = 0; x < hexArray.width; x++)
		for (int y = 0; y < hexArray.height; y++) {;
			if (hexArray.getHexOffset(x, y).cost == 10) {
				glm::vec3 hexPos = hexArray.getHexOffset(x, y).position;
				imRendr::drawLine(hexPos + glm::vec3(-size, 0, 0), hexPos + glm::vec3(size, 0, 0));
				imRendr::drawLine(hexPos + glm::vec3(0, -size, 0), hexPos + glm::vec3(0, size, 0));

		}
	}
	imRendr::setDrawColour({ 1,1,1,1 });

}



/** Flood-fill rooms with pathfinding values for door creation. */
void CProcGen::fillRooms() {
	//for each room, do a breadth-first search for empty hexes.
	for (auto& room : rooms) {
		std::queue<CHex>  frontier;
		frontier.push(room.getOriginHex());
		std::unordered_map<glm::i32vec2,CHex> reached;
		reached[room.getOriginHex().getAxial()] = room.getOriginHex();


		while (!frontier.empty()) {
			CHex current = frontier.front();
			frontier.pop();

			//for each neighbour of this hex
			for (int dir = 0; dir < 6; dir++) {
				CHex& next = getNeighbour(current, (THexDir)dir);

				if (hexArray.getHexCube(next).content == solidHex)
					continue; //solid hex

				if (hexArray.outsideArray(next))
					continue;

				//if next not in reached
				if (reached.find(next.getAxial()) == reached.end()) {
					frontier.push(next);
					reached[next.getAxial()] = next;
					hexArray.getHexCube(next).cost = 10;
				}

			}



		}



	}
}
