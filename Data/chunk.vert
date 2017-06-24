#version 330

layout(location = 0) in vec3 cubeCorner;


uniform float cubeSize;
uniform float LoDscale;
uniform vec3 samplePos; //position of bottom nwcorner of this chunk in sample space.
uniform float samplesPerCube; 

out VertexData {

	vec3 vert;
	float sample;
	
	vec3 opVert;
	float opSample;
} outData;



#include noise.lib

float getSample(vec3 cornerOffset) {
		vec3 sampleCorner = vec3(samplePos ) + ((   (cubeCorner * samplesPerCube)  + cornerOffset)  * LoDscale * 1) ;
	
	
	//Get the noise value at the 2D position of this corner.
	float noise =  octave_noise_2d(5,0.5,1,sampleCorner.x ,sampleCorner.z ); //0.5f;

	noise = pow(noise,2);
		
	//scale the noise a little to create the height at this x,z sample position.
	float surfaceHeight = (noise * 0.3) - 0.3;
	
	
	//clip the surface height against the height of this corner. Values outside 1 mean the surface doesn't intersect this point.
	return sampleCorner.y - surfaceHeight;

}



void main() {
	

	
	outData.vert = cubeCorner * vec3(cubeSize,cubeSize,cubeSize) ;
	outData.opVert = outData.vert + vec3(0,0,cubeSize);
	
	outData.sample = getSample(vec3(0,0,0));
	outData.opSample = getSample(vec3(0,0,samplesPerCube));
	

	
}
