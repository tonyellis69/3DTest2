#version 330

in vec2 vTexCoord;
out vec4 colour;

uniform sampler2D blurTexture;
uniform float fade;


void main() {
	
	vec4 blurColour = texture2D(blurTexture, vTexCoord.st );
	
	colour = blurColour;
	colour.a *= fade;
	
};
