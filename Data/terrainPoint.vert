#version 330
#include noise.lib

layout(location = 0) in vec3 vertPos; //A vertex somewhere in a terrain.

uniform vec3 sampleBase; //Bottom point from which we're offsetting this vertex;
uniform float offsetScale; //Scale we're using to offset this vertex;
out float result; 




void main () {
	//find the position in sample space of this vertex
	vec3 vertSamplePos = sampleBase + (vertPos * offsetScale);

	float noise = terrainValue(5,vertSamplePos); 
	
	
	result = noise;	
}