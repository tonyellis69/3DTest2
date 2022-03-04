#pragma once

#include <glm/glm.hpp>

/** Simple class for storing physics properties. */
class CPhys {
public:


	glm::vec3 moveImpulse = { 0,0,0 }; //Motive force.
	glm::vec3 velocity = { 0,0,0 };
	float invMass = 0;
	float restitution = 0.0f; //bounce
	float drag = 0.00125f;
	//float maxVelocity = 4.0f;

	float boundingRadius = 0;
};

