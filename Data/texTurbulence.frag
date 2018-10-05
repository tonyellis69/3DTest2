#version 330

#include noise.lib

in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 
uniform float frequency;
uniform float power;
uniform int roughness;
 
 

void main() {
	vec2 xOffset = vec2(12414.0 / 65536.0, 65124.0 / 65536.0);
//  z0 = z + (31337.0 / 65536.0);
	vec2 yOffset = vec2(26519.0 / 65536.0, 18128.0 / 65536.0);

	//perturb texcoords by small noise values
	
	vec2 distort;
	distort.x = texCoord0.x + fbm2DclassicFreq(roughness,texCoord0 + vec2(2,2), frequency) * power;
	distort.y = texCoord0.y + fbm2DclassicFreq(roughness,texCoord0  + vec2(1,1),frequency) * power;
	
	//use perturbed coordinates to look up source texture
	vec4 colour = texture(source,distort);
	
	
	FragColour = colour;
	
};