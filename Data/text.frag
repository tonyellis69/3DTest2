#version 330

in vec2 texCoord0;

out vec4 colour;

uniform sampler2D fontTexture;

uniform vec4 textColour;

void main()
{
	float alpha = texture2D(fontTexture, texCoord0).r;
	//alpha = clamp(alpha * 0.8, 0.0, 1.0);
	
	
	colour =  vec4(textColour.rgb, alpha * textColour.a );
	

	//colour = vec4(1, 1, 1, texture2D(fontTexture, texCoord0).r) * textColour;
	

};
