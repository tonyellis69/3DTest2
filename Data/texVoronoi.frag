#version 330

#include noise.lib

in vec2 samplePoint0;
in vec2 texCoord0;

out vec4 FragColour;

uniform float frequency;

uniform bool distance;
uniform bool randomHue;

void main() {

	vec3 result = myVoronoi(samplePoint0 * frequency);
	
	float colour = 0;
	if (distance)
		colour += result.x;
	if (randomHue) {
		colour += result.y;
		if (distance)
			colour -= 0.5; //value over 1.0 otherwise
		
	}
	
	FragColour = vec4(colour,colour,colour,1 );
	
	
};