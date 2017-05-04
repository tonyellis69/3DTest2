#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;


smooth out vec4 theColor;

uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix

uniform vec4 levels;
uniform float domeHeight;
uniform vec3 heightColour[4];


void main()
{
	gl_Position = mvpMatrix * vec4(position,1);
	
	float gradient = pow( position.y,0.5);   //0.1 squashes white down, 0.9 draws it up

	theColor = vec4(mix(heightColour[0],heightColour[3],gradient),1);
	
}
