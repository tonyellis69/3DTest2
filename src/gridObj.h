#pragma once

#define _USE_MATH_DEFINES //for cmath
#include <glm/glm.hpp>

#include "renderer/buf.h"
#include "hex/IHexRenderer.h"
//#include "IHexWorld.h"
#include "hex/hex.h"

#include "hex/lineModel.h"


/** A base class for 3D grid objects. */
class IHexWorld;
class CGridObj {
public:
	CGridObj();
	static void setHexRenderer(IHexRenderer* rendrObj);
	static void setHexWorld(IHexWorld* obj);
	void setZheight(float height);
	void setBuffer(CBuf* buffer);
	void setLineModel(CLineModel& lineModel);
	void setPosition(int x, int y);
	void setPosition(CHex& hex);
	virtual void draw();
	virtual bool update(float dT);
	virtual bool updateMove(float dT);


	CBuf* buf; ///<Identifies the graphics buffer to use for drawing this object 
	glm::mat4 worldMatrix; ///<Position and orientation in the 3D universe.
	glm::vec3 worldPos; ///<Position in world space.
	CLineModel lineModel;

protected:
	void buildWorldMatrix();

	glm::vec3 moveVector;
	float moveSpeed;
	glm::vec3 worldSpaceDestination;

	float prevTargetDist;

	float rotation; ///<Angle of object's z-rotation in world space.
	float zHeight; ///<Height above XY plane where drawn.


	inline static IHexWorld* hexWorld;

private:
	

	inline static IHexRenderer* hexRendr;

	float proximityCutoff;


	glm::vec4 colour;
	THexDraw drawData;
};