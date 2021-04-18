#include "viewField.h"

#include <unordered_set>
#include <set>

#include <glm\glm.hpp>

#include "hex/hex.h"

void CViewFieldArc::setField(int radius, float fovAngleDegrees) {
	this->radius = radius;
	fovAngle = glm::radians(fovAngleDegrees);
}

bool CViewFieldArc::calculateOutline(CHex& pos, float rotation) {
	bool recalc = false;;
	if (pos != apexPos) {
		apexPos = pos;
		recalc = true;
	}

	std::tuple<int, int> targetHex = findRingHex(radius, rotation);
	if (targetHex != currentTargetHex) {
		currentTargetHex = targetHex;
		recalc = true;
	}

	if (recalc) {
		arcHexes = findArc2(apexPos, radius, fovAngle, rotation);
	}

	return recalc;
}

bool CViewFieldArc::searchView(CHex& hex) {
	for (auto viewHex : visibleHexes) {
		if (viewHex == hex) {
			return true;
		}
	}
	return false;
}



//Code for player viewfield //


void CViewFieldCircle::setField(int radius) {
	this->radius = radius;

	//create ring of hexes at radius from centre
	//make this our visible hexes
	ringHexes = *findRing(radius, centre);


}

void CViewFieldCircle::update(CHex& pos) {
	centre = pos;
	ringHexes = *findRing(radius, centre);
}

bool CViewFieldCircle::searchView(CHex& hex) {
	if (hex == centre)
		return false;

	for (auto viewHex : visibleHexes) {
		if (viewHex == hex) {
			return true;
		}
	}
	return false;
}
