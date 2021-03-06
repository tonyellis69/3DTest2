#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 
uniform sampler2D source2; 

void main() {
	vec4 src1 = texture(source,texCoord0);
	vec4 src2 = texture(source2,texCoord0);
	
	vec4 blend = mix(src1,src2,src2.a - src1.a);
	

	FragColour = blend;
	
};

