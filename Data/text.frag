#version 330

in vec2 texCoord0;

out vec4 colour;

uniform sampler2D fontTexture;

uniform vec4 textColour;

void main()
{
	vec4 mask = texture2D(fontTexture, texCoord0.st );
	//mask.rgb = 1 everywhere. mask.a = 1 on font, 0 outside, gradient in between.
	
	colour = mask * textColour;
	
	//problem seems to be with the tinted back panel. When it is totally transparent, blue font edges look fine
	//somehow it's blending wrongly
	
	//it's something to do with the *number* of channels involved. With two on full - any two - the dark outline disappears.
	//something to do with the blend formula? Investigate
	

};
