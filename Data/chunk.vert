#version 330
#include noise.lib
layout(location = 0) in vec3 cubeVertPos; //Relative position of an MC cube vertex within this chunk.


uniform float cubeSize; //The worldspace size of the MC cubes that make up this chunk.
uniform float LoDscale; // This chunk's size multiplyer. Goes 1,2,4,8 etc for LoDs of 1,2,3,4
uniform vec3 samplePos; //position of bottom nwcorner of this chunk in sample space.
uniform float samplesPerCube; //the samplespace size of the MC cubes that make up this chunk. TO DO: change name?

out vertexPair {

	vec3 vert;
	float sample;
	
	vec3 opVert;
	float opSample;
} vertexPairOut;





float getSample(vec3 vertSamplePos) {
	float noise = terrainValue(5, vertSamplePos.xyz ); 
	
	return noise;
}



void main() {
	vertexPairOut.vert = cubeVertPos * vec3(cubeSize,cubeSize,cubeSize); //cube vertex position in worldspace units
	vertexPairOut.opVert = vertexPairOut.vert + vec3(0,0,cubeSize); //opposite cube vertex in worldspace units
	
	float scaledCubeSampleSize = samplesPerCube  * LoDscale;
	vec3 vertSamplePos = samplePos  + ( cubeVertPos * scaledCubeSampleSize) ;
	vertexPairOut.sample = getSample(vertSamplePos);
	
	vec3 opVertSamplePos = vertSamplePos + vec3(0,0,scaledCubeSampleSize);
	vertexPairOut.opSample = getSample(opVertSamplePos);
}
