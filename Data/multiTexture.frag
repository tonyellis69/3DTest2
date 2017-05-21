#version 330

in vec2 texCoord0;

out vec4 colour;

uniform sampler2D mySampler1;
uniform sampler2D mySampler2;

uniform vec2 tile1;
uniform vec2 tile2;

uniform vec2 offset1;
uniform vec2 offset2;

void main()
{
	vec4 colour1 = texture2D(mySampler1, (texCoord0.st * tile1) + offset1);
	vec4 colour2 = texture2D(mySampler2, (texCoord0.st * tile2) + offset2);
	
	colour = mix(colour1,colour2,0.5);
	
};
