#version 330

#include noise2.lib
#line 5

in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source1; //image
uniform sampler2D source2; //texture to cutup
uniform float shades = 8;


void main() {
	
	
	float shade = texture(source1,texCoord0).r;
	float shadeSteps = 1.0/shades;
	vec2 offset = random2(vec2(shade)) * 0.5;
	
	vec2 cutupOffset = offset + texCoord0;
	cutupOffset = fract(cutupOffset);
	FragColour = texture(source2,cutupOffset);
	

};

