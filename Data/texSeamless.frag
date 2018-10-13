#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 


void main() {
	float extent = 0.5f;
	float xExtend = mod(texCoord0.x + extent, 1.0);
	float yExtend = mod(texCoord0.y + extent, 1.0);
	vec4 swPixel = texture(source,texCoord0);
	vec4 sePixel = texture(source, vec2( xExtend,texCoord0.y) );
	vec4 nwPixel = texture(source, vec2(texCoord0.x, yExtend ));
	vec4 nePixel = texture(source, vec2(xExtend, yExtend) );
	
	float mixX =  abs( 1.0 - (texCoord0.x  / extent));
	float mixY =  abs(1.0 - (texCoord0.y  / extent));
	
	

	
	float xBlend = mix(swPixel.r, sePixel.r, mixX);
	float yBlend = mix(nwPixel.r, nePixel.r, mixX);
	
	
	float value = mix(xBlend,yBlend,mixY);
	
	//value = xBlend.r;
	
	FragColour = vec4(value,value,value,1);
	

	
};

