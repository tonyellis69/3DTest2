#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source1; 
uniform sampler2D source2; 
uniform sampler2D map; 
uniform float lowerBound;
uniform float upperBound;
uniform float falloff = 0.5;

void main() {
	float mapValue = texture(map,texCoord0).r;
	
	float control =  smoothstep(lowerBound ,lowerBound + falloff,  mapValue); 
	float control2 =  smoothstep(upperBound  ,upperBound  + falloff,mapValue); 
	control = control - control2;
	
	vec4 colour2 = texture(source2,texCoord0);
	vec4 colour1 = texture(source1,texCoord0);

	vec4 colour = mix(colour1,colour2,control);
	
	FragColour = colour; //vec4(colour,colour,colour,1);	
};

