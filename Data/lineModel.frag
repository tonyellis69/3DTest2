#version 330

uniform vec4 colour = vec4(0,1,0,1);

in vec2 gsTexCoord;

out vec4 outputColor;

void main() {
	float lateralFade = 1 - abs(gsTexCoord.y - 0.5) / 0.5;


	outputColor = colour;
	outputColor.a = lateralFade;

}
