#version 330

out vec4 outputColour;

uniform float lifeTime;
uniform float size;
uniform float timeOut;

in vec4 gsColour;

void main() {
	float fadeStart = 0;// timeOut / 4.0f;

	float fade = smoothstep(fadeStart,timeOut,lifeTime);
	
	//outputColour = vec4(1,1,1,1 - fade);
	outputColour = gsColour;
	outputColour.a = 1.0f - fade;
};