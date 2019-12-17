#pragma once

#include "hex/hexObject.h"

/** A class describing characteristics and behaviour unique to
	the player hex-world object. */
class CPlayerObject : public CHexObject {
public:
	bool update(float dT);
	



};