#version 330


smooth in vec3 fragPos; 
smooth in vec2 texCoord0;
in float diff;
in vec3 viewNorm;
in vec3 lightDir;

uniform sampler2D sample;


struct Light {
	vec3 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};
uniform Light light;

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
}; 
uniform Material material;





out vec4 outputColor;

void main()
{

	
	float specularStrength = 0.25;
	vec3 viewDir = normalize( -fragPos);
	vec3 reflectDir = reflect(-lightDir, viewNorm);
 
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
	vec4 texColour = texture(sample,texCoord0);
	float textureExists = texColour.a;
	
	vec4 diffuse = (1 - textureExists) * material.diffuse + textureExists * texColour;
	vec4 ambient = (1 - textureExists) * material.diffuse + textureExists * texColour;
	
	outputColor = vec4(light.diffuse.xyz * diffuse.xyz * diff,1  )  
				+  vec4(light.ambient.xyz * ambient.xyz,1) 
				+ vec4(spec * light.specular.xyz * material.specular.xyz,1);

				
	//outputColor = texture(sample,texCoord0);

}
