#version 330

in vec2 texCoord0;
in vec2 position0;

out vec4 colour;

uniform sampler2D fontTexture;

uniform vec4 textColour;

uniform float fadeInX = 1.0f; //Position along line at which fade is happening.

void main()
{
	float alpha = texture2D(fontTexture, texCoord0).r;
	
	//uncomment for a great scattered, random character fade-in!
	//float fade = step(fadeInX,texCoord0.x);
	
	//float fade = step(fadeInX,position0.x);
	float fade = smoothstep(fadeInX,fadeInX + 0.25, position0.x);
	
	
	
	
	
	
	////////////////!!!!!!!!!removed while remaking lineBuffer
	//alpha *= 1.0 - fade;
	
	colour =  vec4(textColour.rgb, alpha * textColour.a );
	
	
	
	
	
};
