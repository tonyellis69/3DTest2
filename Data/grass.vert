#version 330

/** Grass vertex shader: takes one position vector from an instanced array, and passes it on to a geometry shader. */

layout(location = 0) in vec3 dummy;  //For now, this takes a dummy variable representing an instanced model, because we create our model internally.
layout(location = 1) in vec3 grassPosition;  //The point at which we'll place our grass.



out vec4 position;


void main() {
	position = vec4(grassPosition,1.0);
}