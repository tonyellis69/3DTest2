#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source1; 
uniform sampler2D source2; 
uniform sampler2D map; 
uniform float lowerBound;
uniform float upperBound;
uniform float falloff = 0.03;

void main() {
	float mapValue = texture(map,texCoord0).r;
	
	float control =  smoothstep(lowerBound ,lowerBound + falloff,  mapValue); 
	float control2 =  smoothstep(upperBound - falloff ,upperBound ,mapValue); 
	control = control - control2;
	
	float colour2 = texture(source2,texCoord0).r;
	float colour1 = texture(source1,texCoord0).r;

	float colour = mix(colour1,colour2,control);
	
	FragColour = vec4(colour,colour,colour,1);	
};

