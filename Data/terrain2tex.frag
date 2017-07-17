#version 330

//layout(origin_upper_left) 
//gl_FragCoord;

layout(location = 0) out vec4 colour;

uniform vec2 nwSampleCorner; //The position in sample space of the nw corner of our terrain.
uniform float pixelScale; //ratio of sample space to screen space.

#include noise.lib

void main() {
	int octaves = 5;
	float persistence = 0.5f;
	float xzScaling = 2;	
	float startingAmplitude = 0.22f; //0.5f;//0.22f;
		
	float noise = terrainHeight(octaves,persistence,xzScaling, startingAmplitude, nwSampleCorner +  (gl_FragCoord.xy  * pixelScale)); 

	
	noise = (noise * 0.5f) + 0.5f; //put in 0-1 range
	
	

	//noise += 0.2f;
	//noise *= 2;
	
	noise =  1- noise;

	colour = vec4(noise,noise,noise,1);
}