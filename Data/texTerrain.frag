#version 330

in vec2 texCoord0;  

uniform sampler2D source; 
uniform int gridSize = 10;

out vec4 FragColour;

void main() {  
	
	
	FragColour = vec4(0,0,0,1);
	
	float cellValue = texture(source,texCoord0).r;
	
	if (cellValue < 0)
		FragColour = vec4(0.0,0.0,0.5,1);
	else 
	if (cellValue == 1.0)
		FragColour = vec4(0,1,0,1);
	else
	if (cellValue == 2.0)
		FragColour = vec4(1,0,0,1);
	else
		if (cellValue == 3.0)
			FragColour = vec4(1,1,0,1);
	else
		FragColour = vec4(vec3(cellValue),1);
		
	vec2 point = texCoord0 * gridSize;
	point = fract(point);
	
	FragColour += (step(0.98, point.x) + step(0.98, point.y));	
		
}