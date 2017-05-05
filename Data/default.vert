#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;


smooth out vec4 fragColour;

uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix

uniform mat3 normalModelToCameraMatrix;
uniform vec3 lightDirection;
uniform vec4 lightIntensity;
uniform vec4 ambientLight;
uniform vec4 colour;

void main()
{
	gl_Position = mvpMatrix * vec4(position,1);
	
	vec3 normCamSpace = normalize(normalModelToCameraMatrix * vec3(normal));
	
	float cosAngIncidence = dot(normCamSpace, (lightDirection));
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);

	fragColour = (lightIntensity * colour * cosAngIncidence)
				+ (colour * ambientLight );
	
	
}
