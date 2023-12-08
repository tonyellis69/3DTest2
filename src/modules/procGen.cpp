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

	if (!indiRects.empty()) {
		for (auto& indiRect : indiRects) {
			indiRect.drawWireFrame();
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
		if (e.key >= '1' && e.key <= '9') {
			if (mstEdges.empty())
				return;
			int startRoom = e.key - 49;
			findPathDijkstra(mstEdges[startRoom].a, mstEdges[startRoom].b);
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
		if (e.key == 'S') {
			solveDoorPaths();
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
	indiRects.clear();
	doorBots.clear();
	hexArray.clear();
	gameWorld.updateHexMap(hexArray);
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
	indiRects.clear();
	for (auto& edge : mstEdges) {
		CDoorRect doorRect(rooms[edge.a], rooms[edge.b]);
		if (doorRect.tooSmall)
			 createIndiRect(doorRect);
		else
			doorRects.push_back(doorRect);

	}
}

/** Create an CIndiRect to replace the given doorRect. */
void CProcGen::createIndiRect(CDoorRect& failDoorRect) {
	//shared vec should be 2, is it?
	CIndiRect A(failDoorRect.roomA, failDoorRect.roomB);
	CIndiRect B(failDoorRect.roomB, failDoorRect.roomA);

	fitIndiRect(A);
	fitIndiRect(B);

	//pick best
	//if none, freak out 
	//if (A.scrapped && B.scrapped)
	//	return; ///FAIL!!!

	if (A.scrapped) {
		indiRects.push_back(B);
		return;
	}
	if (B.scrapped) {
		indiRects.push_back(A);
		return;
	}

	if (A.volume() > B.volume())
		indiRects.push_back(B);
	else
		indiRects.push_back(A);

}

void CProcGen::fitIndiRect(CIndiRect& indiRect) {
	//clip indiRect against existing doors
	for (auto& room : rooms) {
		indiRect.clipAgainstRoom(room);
		if (indiRect.scrapped)
			return;
	}


	if ( (indiRect.width() < hexWidth * 3) || (indiRect.height() < hexRowHeight * 4) ) {
		indiRect.scrapped = true;
		return;
	}

	//scrap if there's a clash with an existing indiRect
	for (auto& existingRect : indiRects) {
		if (existingRect.overlap(indiRect)) {
			indiRect.scrapped = true;
			return;
		}
	}

	//or an existing doorRect
	for (auto& doorRect : doorRects) {
		if (doorRect.overlap(indiRect)) {
			indiRect.scrapped = true;
			return;
		}
	}
}

/** Write rooms as hexes to our hexArray. */
void CProcGen::drawHexes() {
	for (auto& room : rooms) {
		room.writeHexes(hexArray);
	}
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

	//draws cost:
	//for (int x = 0; x < hexArray.width; x++)
	//	for (int y = 0; y < hexArray.height; y++) {
	//		if (hexArray.getHexOffset(x, y).cost == 0)
	//			continue;
	//		if (hexArray.getHexOffset(x, y).cost == 1)
	//			imRendr::setDrawColour({ 1,1,1,0.75f });
	//		if (hexArray.getHexOffset(x, y).cost == 2)
	//			imRendr::setDrawColour({ 0,0,1,1 });
	//		if (hexArray.getHexOffset(x, y).cost == 3)
	//			imRendr::setDrawColour({ 1,0,0,0.75f });
	//		if (hexArray.getHexOffset(x, y).cost == 4)
	//			imRendr::setDrawColour({ 1,0,0,1 });

	//		glm::vec3 hexPos = hexArray.getHexOffset(x, y).position;
	//		imRendr::drawLine(hexPos + glm::vec3(-size, 0, 0), hexPos + glm::vec3(size, 0, 0));
	//		imRendr::drawLine(hexPos + glm::vec3(0, -size, 0), hexPos + glm::vec3(0, size, 0));
	//	}


	//draws came-from:
	for (auto& hex : cameFrom) {
		CHex currentHex = axialToCube(hex.first.x, hex.first.y);

		switch (hexArray.getHexCube(currentHex).cost) {
		case 1: imRendr::setDrawColour({ 1,1,1,0.75f }); break;
		case 2: imRendr::setDrawColour({ 0,0,1,1 }); break;
		case 3: imRendr::setDrawColour({ 1,0,0,0.75f }); break;
		case 4: imRendr::setDrawColour({ 1,0,0,1 }); break;
		}

		CHex cameFromHex = hex.second;
		THexDir dir = relativeDir(currentHex, cameFromHex);
		glm::vec3 pt = moveVector3D[dir] * 0.5f;;
		glm::vec3 hexPos = cubeToWorldSpace(currentHex);
		imRendr::drawLine(hexPos , hexPos + pt);

	}

	
	imRendr::setDrawColour({ 1,1,1,1 });

}

/** Give room hexes useful cost values for pathfinding. */
void CProcGen::fillRooms() {
	int roomNo = 0;
	for (auto& room : rooms) {

		std::queue<CHex>  frontier;
		frontier.push(room.getOriginHex());
		std::unordered_map<glm::i32vec2, CHex> reached;

		while (!frontier.empty()) {
			CHex current = frontier.front();
			frontier.pop();

			//for each neighbour of this hex
			for (int dir = 0; dir < 6; dir++) {
				CHex& next = getNeighbour(current, (THexDir)dir);

				if (hexArray.getHexCube(next).content == solidHex) {
					hexArray.getHexCube(next).cost = 2;
					continue; //stop at solid hexes
				}

				if (hexArray.outsideArray(next))
					continue;

				if (reached.find(next.getAxial()) == reached.end()) {
					frontier.push(next);
					reached[next.getAxial()] = next;
					hexArray.getHexCube(next).cost = 1;
					hexArray.getHexCube(next).roomNo = roomNo;
				}

			}

		}
		roomNo++;


	}

	//ensure corners are verbotten
	for (auto& room : rooms) {
		THexList corners = room.getCornerHexes();
		for (auto& corner : corners) {
			hexArray.getHexCube(corner).cost = 4;
			for (int dir = 0; dir < 6; dir++)
				hexArray.getHexCube(getNeighbour(corner, (THexDir)dir)).cost = 4;					
		}
	}

}



/** Map the paths from A to B, with their costs. */
void CProcGen::findPath(int roomA, int roomB) {

	CProcRoom& startRoom = rooms[roomA];
	CProcRoom& endRoom = rooms[roomB];
	CHex endHex = endRoom.getOriginHex();

	std::queue<CHex>  frontier;
	frontier.push(startRoom.getOriginHex());
	//std::unordered_map<glm::i32vec2, CHex> reached;
	//reached[startRoom.getOriginHex().getAxial()] = startRoom.getOriginHex();
	cameFrom.clear();

	while (!frontier.empty()) {
		CHex current = frontier.front();
		frontier.pop();

		if (current == endHex)
			break;

		//for each neighbour of this hex
		for (int dir = 0; dir < 6; dir++) {
			CHex& next = getNeighbour(current, (THexDir)dir);

			if (hexArray.getHexCube(next).content == solidHex) {
				//continue; //solid hexes never get added to the frontier
			}

			if (hexArray.outsideArray(next))
				continue;

			if (cameFrom.find(next.getAxial()) == cameFrom.end()) {
				frontier.push(next);
				cameFrom[next.getAxial()] = current;
			}
		}
	}
}

void CProcGen::findPathDijkstra(int roomA, int roomB) {

	CProcRoom& startRoom = rooms[roomA];
	CProcRoom& endRoom = rooms[roomB];
	CHex endHex = endRoom.getOriginHex();

	std::priority_queue<PQElement, std::vector<PQElement>, order> frontier;
	frontier.push({ 0,startRoom.getOriginHex() });
	std::unordered_map<glm::i32vec2, int> costSoFar;
	costSoFar[startRoom.getOriginHex().getAxial()] = 0;
	cameFrom.clear();
	//cameFrom[startRoom.getOriginHex().getAxial()] = startRoom.getOriginHex();

	while (!frontier.empty()) {
		CHex current = frontier.top().second;
		frontier.pop();



		if (current == endHex)
			break;

		int roomNo = hexArray.getHexCube(current).roomNo;
		if (roomNo != -1 && roomNo != roomA && roomNo != roomB)
			continue;

		if (hexArray.getHexCube(current).cost == 4)
			continue;




		//for each neighbour of this hex
		for (int dir = 0; dir < 6; dir++) {
			CHex& next = getNeighbour(current, (THexDir)dir);



			if (hexArray.outsideArray(next))
				continue;

			if (hexArray.getHexCube(next).cost == 4)
				continue; //why is this necessary?

	
			//disqualify solid hexes if there isn't a neighbour hex we can also remove (doors being 2 hexes wide) 
			if ( hexArray.getHexCube(next).content == solidHex) {
				if (hexArray.getHexCube(getNeighbour(next,hexEast)).cost == 4 ||
					hexArray.getHexCube(getNeighbour(next, hexSE)).cost == 4 ||
					hexArray.getHexCube(getNeighbour(next, hexSW)).cost == 4) {
					continue;
				}
			}

			int newCost = costSoFar[current.getAxial()] + hexArray.getHexCube(next).cost;

			if (costSoFar.find(next.getAxial()) == costSoFar.end() || newCost < costSoFar[next.getAxial()]) {
				costSoFar[next.getAxial()] = newCost;
				frontier.push({ newCost,next });
				cameFrom[next.getAxial()] = current;
			}
		}
	}
}

void CProcGen::solveDoorPaths() {
	for (auto& roomPair : mstEdges) {
		findPathDijkstra(roomPair.a, roomPair.b);
		CHex currentHex = rooms[roomPair.b].getOriginHex();
		bool horizontalTravel = false;
		while (currentHex != rooms[roomPair.a].getOriginHex()) {
			if (currentHex == CHex(-4, 9, -5))
				int b = 0;


			CHex comeFromHex = cameFrom[currentHex.getAxial()];
			THexDir fromDir = neighbourDirection(currentHex,comeFromHex);
			 

			if (hexArray.getHexCube(currentHex).content == solidHex || hexArray.getHexCube(currentHex).content == testHex) {
	
					if (hexArray.getHexCube(getNeighbour(currentHex, hexEast)).content == solidHex) {
						hexArray.getHexCube(getNeighbour(currentHex, hexEast)).content = testHex;
					}
					if (hexArray.getHexCube(getNeighbour(currentHex, hexSE)).content == solidHex) {
						hexArray.getHexCube(getNeighbour(currentHex, hexSE)).content = testHex;
					}
					if (hexArray.getHexCube(getNeighbour(currentHex, hexSW)).content == solidHex) {
						hexArray.getHexCube(getNeighbour(currentHex, hexSW)).content = testHex;
					}
				

			}




			horizontalTravel = (fromDir == hexEast || fromDir == hexWest) ? true : false;
			if (horizontalTravel) {
				CHex NW = getNeighbour(currentHex, hexNW);
				if (hexArray.getHexCube(NW).roomNo == -1 && hexArray.getHexCube(NW).content != solidHex)
					hexArray.getHexCube(NW).content = testHex2;
				CHex NE = getNeighbour(currentHex, hexNE);
				if (hexArray.getHexCube(NE).roomNo == -1 && hexArray.getHexCube(NE).content != solidHex)
					hexArray.getHexCube(NE).content = testHex2;
			}
			else {
				CHex W = getNeighbour(currentHex, hexWest);
				if (hexArray.getHexCube(W).roomNo == -1 && hexArray.getHexCube(W).content == emptyHex)
					hexArray.getHexCube(W).content = testHex2;
				CHex E = getNeighbour(currentHex, hexEast);
				E = getNeighbour(E, hexEast);
				if (hexArray.getHexCube(E).roomNo == -1 && hexArray.getHexCube(E).content == emptyHex)
					hexArray.getHexCube(E).content = testHex3;


			}

			hexArray.getHexCube(currentHex).content = testHex;
			currentHex = cameFrom[currentHex.getAxial()];
		}
	}

}
