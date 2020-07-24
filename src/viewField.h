#pragma once

#include "hex/hex.h"


/** Defines and maintains the field of view of an entity on a hex grid. */
class CViewField {
public:

	void setField(int radius, float fovAngleDegrees);
	bool update(CHex& pos, float rotation);
	bool searchView(CHex& hex);

	THexList arcHexes;
	THexList visibleHexes;

private:
	void calcField(THexList& arcHexes);

	CHex apexPos = { -1,-1,-1 };

	int radius;
	float fovAngle;

	std::tuple<int, int> currentTargetHex = { -1,-1 };
};