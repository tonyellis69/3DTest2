#version 330

in vec2 vTexCoord;
out vec4 colour;

uniform sampler2D mapTexture;
uniform sampler2D modelsTexture;



void main() {
	
	vec4 mapColour =  texture2D(mapTexture, vTexCoord.st );
	vec4 modelsColour =  texture2D(modelsTexture, vTexCoord.st );
	
	colour = mapColour + modelsColour;
};
