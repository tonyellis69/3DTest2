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

	
	
	int octaves = 5;
	float persistence = 0.5f;
	float xzScaling = 2;	
	float startingAmplitude = 0.22f; //0.5f;//0.22f;

	
	float noise = terrainHeight(octaves,persistence,xzScaling, startingAmplitude, vertSamplePos.xyz ); 
	
	
	
	


	//clip the surface height against the height of this corner. Values outside 1 mean the surface doesn't intersect this point.
	float vertSample = vertSamplePos.y - noise;

	outData.vertSample = vertSample; // output the result to the geometry shader

}