#version 330

out vec4 outputColour;

uniform float lifeTime;
uniform float size;
uniform float timeOut;

void main() {
	float fadeStart = timeOut / 4.0f;

	float fade = smoothstep(fadeStart,timeOut,lifeTime);
	
	outputColour = vec4(1,1,1,1 - fade);
};