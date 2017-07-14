#version 330


uniform vec4 colour1;
uniform vec4 colour2;
in vec2 fragTexCoord;
out vec4 outputColor;

void main()
{
	outputColor = mix(colour1,colour2,fragTexCoord.y);
}