#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 
uniform sampler1D palette; 

void main() {
	vec4 src = texture(source,texCoord0);
	vec4 colour = texture(palette,src.r);

	FragColour = colour;
	
};

