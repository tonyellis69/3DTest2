#version 330

#include noise.lib

in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 
uniform float scale;
uniform float bias;


void main() {
	vec4 colour = texture(source,texCoord0);
	float value = colour.r;
	
	value = value * 2.0 - 1.0;
	
	value = value * scale + bias;
	
	value = value * 0.5 + 0.5;
	
	
	FragColour = vec4(value,value,value,colour.a);
	
};

