#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 
uniform sampler2D source2; 

void main() {
	vec4 src1 = texture(source,texCoord0);
	vec4 src2 = texture(source2,texCoord0);
	
	float value = (src1.r  * 0.5) + (src2.r * 0.5);
	//value = src2.r;

	FragColour = vec4(value,value,value,1);
	
};

