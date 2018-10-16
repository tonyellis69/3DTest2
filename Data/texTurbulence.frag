#version 330

#include noise.lib

in vec2 samplePoint0;
in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 
uniform float frequency;
uniform float power;
uniform int roughness;
 
 

void main() {

	//perturb texcoords by small noise values
	
	vec2 distort;
	distort.x = texCoord0.x + fbm2DclassicFreq(roughness,texCoord0 , frequency) * power;
	distort.y = texCoord0.y + fbm2DclassicFreq(roughness,texCoord0  + vec2(4,4),frequency) * power;
	
	//use perturbed coordinates to look up source texture
	vec4 colour = texture(source,distort);
	
	
	FragColour = colour;
	
};