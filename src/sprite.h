#pragma once

#include <glm/glm.hpp>
#include "lineModel.h"

/** Basic sprite class. A sprite here is a drawable thing exisiting
	in worldspace, that gets updated in realtime. */
class CSprite {
public:
	CSprite();
	virtual void update(float dT) = 0;
	virtual void draw() = 0;
	virtual void setPosition(glm::vec3& pos);
	virtual void setPosition(glm::vec3& pos, float rotation);




	glm::mat4* worldMatrix; ///<Position and orientation in the 3D universe.
	glm::vec3 worldPos; ///<Position in world space.
	float rotation;
	CLineModel lineModel;

	bool killMe = false;

protected:
	void buildWorldMatrix();

	float dT;

private:

};