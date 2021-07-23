#version 330

in vec2 texCoord0;

out vec4 colour;

uniform sampler2D mySampler;


uniform vec2 tile = vec2(1,1);


uniform vec2 offset = vec2(0);;

void main()
{
	vec4 colour1 = texture2D(mySampler, (texCoord0.st * tile) + offset);
	
	
	colour = colour1;
	//colour = vec4(1,0,0,1);
};
