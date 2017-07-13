#version 330

smooth in vec4 fragColour;

out vec4 outputColor;

void main()
{
	outputColor = fragColour;
}