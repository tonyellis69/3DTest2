#pragma once

#include "hex/hex.h"


/** Defines and maintains the field of view of an entity on a hex grid. */
class CViewFieldArc {
public:

	void setField(int radius, float fovAngleDegrees);
	bool calculateOutline(CHex& pos, float rotation);
	bool searchView(CHex& hex);

	THexList arcHexes;
	THexList visibleHexes;

private:
	CHex apexPos = { -1,-1,-1 };

	int radius;
	float fovAngle;

	std::tuple<int, int> currentTargetHex = { -1,-1 };
};

class CViewFieldCircle {
public:
	void setField(int radius);
	void update(CHex& pos);
	bool searchView(CHex& hex);

	THexList ringHexes;
	THexList visibleHexes;

	CHex centre;

private:

	int radius;
	
};