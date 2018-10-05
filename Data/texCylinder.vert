#version 330

layout(location = 0) in vec2 position;


uniform mat4 matrix; 


out vec2 texCoord0;
out vec3 samplePoint0;



void main()
{
	gl_Position = vec4(position,0,1);
	samplePoint0 =  vec3(matrix *  gl_Position);   //position;
	
};
