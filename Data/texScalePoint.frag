#version 330

#include noise.lib

in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 
uniform vec3 scale;



void main() {
	vec4 colour = texture(source ,texCoord0 * scale.xy);
	
	float value = colour.r;

	FragColour = vec4(value,value,value,colour.a);
	
};

