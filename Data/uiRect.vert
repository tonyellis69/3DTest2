#version 330 core


layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;

smooth out vec4 fragColour;

uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix


void main()
{
	gl_Position =  mvpMatrix * vec4(vertex.x,vertex.y,0,1);
	
	
	fragColour = colour;
}
