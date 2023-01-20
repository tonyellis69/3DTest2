#pragma once
#define _USE_MATH_DEFINES

#include "entity\entity.h"
#include "roboState.h"
#include "hexRender/multiDraw.h"

enum TRobotState { robotLightSleep,	robotWander3, robotCharge3,
	robotMelee3, robotCloseAndShoot, robotDoNothing
};



enum TLungeState { preLunge, lunging, returning };

/** A class describing basic robot characteristics and
	behaviour. */

class CRobot : public CEntity {
public:
	CRobot();
	void update(float dT);
	void receiveDamage(CEntity& attacker, int damage);
	void fireMissile(CEntity* target);
	
	int hp = 3; 

	
	};


