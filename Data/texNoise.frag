#version 330

#include noise.lib

in vec2 texCoord0;
in vec2 samplePoint0;

out vec4 FragColor;

uniform int octaves;

void main() {
	float noise = fbm3Dclassic(octaves, vec3(samplePoint0,0));
	noise = noise * 0.5 + 0.5;
	FragColor = vec4(noise,noise,noise,1 );
	
};