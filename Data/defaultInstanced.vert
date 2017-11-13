#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 instancePos; //Where to place this particular instance


smooth out vec4 fragColour;
smooth out vec4 worldPos;

uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix

uniform mat3 normalModelToCameraMatrix;
uniform vec3 lightDirection;
uniform vec4 lightIntensity;
uniform vec4 ambientLight;
uniform vec4 colour;

void main()
{
	gl_Position = mvpMatrix * vec4(position + instancePos,1);
	gl_Position.y += 0.9f;
	worldPos = vec4(position,1);
	
	
	vec3 normCamSpace = normalize(normalModelToCameraMatrix * vec3(normal));
	
	float cosAngIncidence = dot(normCamSpace, (lightDirection));
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);
	vec4 myColour = vec4(mod(instancePos.xz,1),0.1+0.5*sin(1.0),1.0);
	fragColour = (lightIntensity * myColour * cosAngIncidence)
				+ (myColour * ambientLight );
	
	
}
