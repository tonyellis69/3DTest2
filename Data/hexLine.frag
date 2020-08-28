#version 330

uniform vec4 colour = vec4(0,1,0,1);

in float outFog;

out vec4 outputColor;

void main() {
	outputColor = colour * (1.0f - outFog);


}
