#version 330
#include noise.lib

layout(location = 0) in vec3 vertPos; //A vertex somewhere in a terrain.

uniform vec3 sampleBase; //Bottom point from which we're offsetting this vertex;
uniform float offsetScale; //Scale we're using to offset this vertex;
out float result; 




void main () {
	//find the position in sample space of this vertex
	vec3 vertSamplePos = sampleBase + (vertPos * offsetScale);

	int octaves = 5;
	float persistence = 0.5f;
	float xzScaling = 2;	
	float startingAmplitude = 0.22f; //0.5f;//0.22f;

	
	float noise = terrainHeight(octaves,persistence,xzScaling, startingAmplitude, vertSamplePos.xyz ); 
	
	

	//clip the surface height against the height of this corner. Values outside 1 mean the surface doesn't intersect this point.
	result = vertSamplePos.y - noise;

	
}