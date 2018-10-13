#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 
uniform sampler2D source2; 

void main() {
	vec4 src1 = texture(source,texCoord0);
	vec4 src2 = texture(source2,texCoord0);
	
	
	src1 = src1 * 2.0 - 1.0;
	src2 = src2 * 2.0 - 1.0;
	
	
	float value = src1.r + src2.r;
	
	value = value * 0.5 + 0.5;
	
	FragColour = vec4(value,value,value,1);
	
};

