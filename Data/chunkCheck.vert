#version 330


//A vertex somewhere on the outer layer of a chunk
layout(location = 0) in vec3 vertPos;

//The position in sample space of the bottom nw corner of the chunk we're checking 
uniform vec3 nwSamplePos;
uniform float LoDscale;

out VertexData {
	float vertSample; //The sample value at this vertex, passed on to the geometry shader.
} outData;


#include noise.lib

void main () {
	//find the position in sample space of this vertex
	vec3 vertSamplePos = nwSamplePos + (vertPos * LoDscale);

	float noise = terrainValue(5,vertSamplePos.xyz ); 

	outData.vertSample = noise; // output the result to the geometry shader
}