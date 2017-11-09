#version 330

in vec2 texCoord0;

out vec4 colour;

uniform sampler2D fontTexture;

uniform vec4 textColour;

void main()
{
	vec4 mask = texture2D(fontTexture, texCoord0.st );
	
	colour = mask * textColour;
	

};
