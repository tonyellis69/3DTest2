#include "viewField.h"

#include <unordered_set>
#include <set>

#include <glm\glm.hpp>

#include "hex/hex.h"

void CViewField::setField(int radius, float fovAngleDegrees) {
	this->radius = radius;
	fovAngle = glm::radians(fovAngleDegrees);
}

bool CViewField::update(CHex& pos, float rotation) {
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
		arcHexes = *findArc2(apexPos, radius, fovAngle, rotation);
	}

	return recalc;
}

bool CViewField::searchView(CHex& hex) {
	for (auto viewHex : visibleHexes) {
		if (viewHex == hex) {
			return true;
		}
	}
	return false;
}

/** Find all hexes in field by tracing lines from the apex to the arc. */
void CViewField::calcField(THexList& arcHexes) {
	std::unordered_set<CHex, hex_hash> uniqueHexes;

	for (auto arcHex : arcHexes) {
		THexList line = *hexLine(apexPos, arcHex);
		uniqueHexes.insert(line.begin() +1, line.end());
	}

	arcHexes.assign(uniqueHexes.begin(),uniqueHexes.end());
}
