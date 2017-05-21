#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;



out vec2 texCoord0;

uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix



void main()
{
	gl_Position = mvpMatrix * vec4(position,1);
	
	texCoord0 = texCoord;
	
};
