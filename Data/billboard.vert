#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;



out vec2 texCoord0;

uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix
uniform vec3 centrePos; //centre point of billboard
uniform mat4 camWorldMatrix; //transforms camera into its place in world space
uniform float size; //dimensions of the billboard

void main()
{
	vec3 upVector = vec3(camWorldMatrix[1]);
	vec3 rightVector = vec3(camWorldMatrix[0]);

	
	//vec3 projCentre =  mvpMatrix * vec4(centrePos,1);
	
	vec3 newPosition = (texCoord.x - 0.5f) * rightVector *  size;
		newPosition +=  (texCoord.y - 0.5f) * upVector  * size;
	

	gl_Position = mvpMatrix * vec4(newPosition,1);
	
	
	
	texCoord0 = texCoord;
	
};
