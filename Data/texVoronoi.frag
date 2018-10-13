#version 330

#include noise.lib

in vec2 samplePoint0;
in vec2 texCoord0;

out vec4 FragColour;

uniform float frequency;

 

void main() {

	//vec2 voronoi =  cellular(samplePoint0 * frequency);
	
	//float f1 = voronoi.x;
	
	
	float f1 = myVoronoi(samplePoint0 * frequency);
	
	
	//f1 = f1 * 0.5 + 0.5;
	FragColour = vec4(f1,f1,f1,1 );
	
	
};