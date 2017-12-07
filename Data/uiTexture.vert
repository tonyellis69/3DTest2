#version 330

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 texCoord;

out vec2 texCoordFrag;
uniform mat4 orthoMatrix; 
uniform vec2 offset;


void main()
{
	gl_Position = orthoMatrix * vec4(vertex + offset,0,1);
	texCoordFrag = texCoord;
	
};
