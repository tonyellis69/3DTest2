#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;


smooth out vec3 fragPos;
smooth out vec2 texCoord0;
out float diff;
out vec3 viewNorm;
out vec3 lightDir;

struct Light {
	vec3 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};
uniform Light light;




uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix
uniform mat4 model;
uniform mat4 view;

uniform vec3 lightDirection = vec3(0,-0.7,-0.7);



void main()
{
	gl_Position = mvpMatrix * vec4(position,1);
	fragPos =  ( view * model * vec4(position,1)).xyz;

	
	vec3 lightPos =  (view * vec4(light.position,1)).xyz;
	lightDir = normalize(lightPos - fragPos);
	

	//viewNorm = mat3(transpose(inverse(view * model))) * normal; 
	//TO DO: may need the above if I do any scaling/skewing (in which case, calculate
	//the transpose outside the shader 
	viewNorm = mat3(view * model) * normal; 
	viewNorm = normalize(viewNorm);
	
	
	diff = dot(viewNorm, lightDir); 
	diff = clamp(diff, 0, 1); //because if light behind surface we get a negative.

	
	texCoord0 = texCoord;
}
