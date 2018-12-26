#version 330

#include noise.lib

in vec2 texCoord0;
in vec2 samplePoint0;

out vec4 FragColor;

uniform int octaves;
uniform float frequency;

in vec3 samplePos0;
in vec3 sampleSize0;


void main() {

	
	float noise =  fbm2DclassicFreq(octaves,samplePoint0, frequency);
	
	

	noise = noise * 0.5 + 0.5;
	FragColor = vec4(noise,noise,noise,1 );
	
};