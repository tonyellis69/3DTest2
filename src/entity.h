#pragma once

#define _USE_MATH_DEFINES //for cmath

#include <glm/glm.hpp>

#include "hex/hex.h"
#include "lineModel.h"
#include "physics/phys.h"


/// <summary>
/// ////////////

/// </summary>

struct TFov { //describes a fov shape
	CHex A;
	CHex B;
	THexList arc;
};


/** The base class for objects in the game world.*/
class CEntity  {
public:
	CEntity();
	virtual ~CEntity() {}
	virtual void update(float dT) {}

	void setPosition(CHex& hex);
	virtual void setLineModel(const std::string& name);
	void setHexDirection(THexDir direction);
	virtual void draw();
	THexDir getDirection() {
		return facing;
	}
	void buildWorldMatrix();

	virtual void receiveDamage(CEntity& attacker, int damage) {};
	virtual void onMovedHex() {};
	virtual std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB) {
		return std::tuple<bool, glm::vec3>();
	}
	float orientationTo(glm::vec3& targetPos);
	void updatePos(glm::vec3& dPos);

	virtual void approachDestHex() {}; //temp! scrap this with per-hex movement
	
	float dT;

	CLineModel lineModel;

	CHex hexPosition; ///<Position on a hex grid in cube coordinates.
	THexDir facing = hexEast; ///<Direction entity is facing.
	float rotation = 0.0f; ///<Angle of object's z-rotation in world space.
	glm::vec3 worldPos = { 0,0,0 }; ///<Position in world space.
	glm::mat4* worldMatrix; ///<Position and orientation in the 3D universe.

	THexList travelPath; ///<Route for movement.
	CHex destination; ///<The hex we're travelling to.

	//components
	CPhys physics;
	
	bool isItem = false; //temp cludge!
	bool isRobot = false; //temp cludge!
	bool deleteMe = false;


};

using TEntity = std::shared_ptr<CEntity>;
using TEntities = std::vector<std::shared_ptr<CEntity>>;


