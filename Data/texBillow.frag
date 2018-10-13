#version 330

#include noise.lib

in vec2 samplePoint0;
in vec2 texCoord0;

out vec4 FragColour;


uniform float frequency;
uniform int octaves;
uniform float persistence;
 

void main() {

	float noise =  billow2D(octaves,samplePoint0, frequency, persistence);

	noise = noise * 0.5 + 0.5;
	FragColour = vec4(noise,noise,noise,1 );
	
	
};