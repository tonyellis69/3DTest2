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

  vec2 adj1 = vec2(12414.0 / 65536.0,65124.0 / 65536.0);
  vec2 adj2 = vec2(26519.0 / 65536.0, 18128.0 / 65536.0);


	//perturb texcoords by small noise values
	
	vec2 distort;
	distort.x = texCoord0.x + fbm2DclassicFreq(roughness,samplePoint0 , frequency) * power;
	distort.y = texCoord0.y + fbm2DclassicFreq(roughness,samplePoint0  + vec2(4,4),frequency) * power;
	
	//use perturbed coordinates to look up source texture
	//distort = mod(distort,1.0);
	vec4 colour = texture(source,distort);
	
	
	FragColour = colour;
	
};