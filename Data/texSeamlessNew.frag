#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 

uniform float percentage;
uniform float falloff;

void main() {
	vec4 colour = texture(source,texCoord0);
	
	
	//centre axis for simplicity
	vec2 pixel = texCoord0 - vec2(0.5,0.5);
	
	float gradientx = 0;
	if ((pixel.x)  > 0.4) {
		gradientx = 1 - ((0.5 - abs(pixel.x)) * 10);
		pixel.x = -pixel.x;
	}
	
	float gradienty = 0;
	if ((pixel.y)  > 0.4) {
		gradienty = 1 - ((0.5 - abs(pixel.y)) * 10);
		pixel.y = -pixel.y;
	}
	
	pixel += vec2(0.5,0.5);
	
	vec4 seamColour = texture(source,pixel);
	
	colour = mix(colour,seamColour,max(gradientx,gradienty) );
	
//	colour = vec4(gradient,0,0,1);
	
	FragColour = colour;
	

	
};

