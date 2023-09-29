#include "doorBot.h"

#include "renderer/imRendr/imRendr.h"

/** For diagnostic purposes only.*/ 
void CDoorBot::drawPos() {
	glm::vec3 pos = cubeToWorldSpace(currentHex);

	float size = 0.5f;

	imRendr::setDrawColour({ 1,1,0,1 });
	imRendr::drawLine(pos + glm::vec3(-size, 0, 0), pos + glm::vec3(size, 0, 0));
	imRendr::drawLine(pos + glm::vec3(0, -size, 0), pos + glm::vec3(0, size, 0));
	imRendr::setDrawColour({ 1,1,1,1 });

}

/** Called for each step. */
void CDoorBot::update() {

	if (currentHex == endHex)
		return;

	THexDir travelDir = relativeDir(currentHex, endHex);
	currentHex = getNeighbour(currentHex, travelDir);
}
