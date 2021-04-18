#version 330 core


layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 texCoord;


uniform mat4 orthoMatrix; //2D to OpenGL orthographic view
out vec2 fragTexCoord;

void main()
{
	gl_Position =  orthoMatrix * vec4(vertex.xy,0,1);
	fragTexCoord = texCoord;
}
