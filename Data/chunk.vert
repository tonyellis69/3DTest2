#version 330

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



#include noise.lib

float getSample(vec3 vertSamplePos) {
	int octaves = 5;
	float persistence = 0.5f;
	float xzScaling = 2;	
	float startingAmplitude = 0.22f; //0.5f;//0.22f;
		
	float noise = terrainHeight(octaves,persistence,xzScaling, startingAmplitude, vertSamplePos.xz ); 
	
	//clip the noise against our y position in the volume. Values outside 1 mean the surface doesn't intersect this point.
	//TO DO: kind of arbitary. Need to find a better way to do this.
	return vertSamplePos.y - noise;
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
