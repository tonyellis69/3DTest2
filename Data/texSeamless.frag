#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 

uniform float percentage;
uniform float falloff;

void main() {
	float seamWidth = percentage / 100;
	vec4 pixel = texture(source,texCoord0);
	vec2 seamCoord = texCoord0;
	float mergeX = 0;
	float mergeY = 0;
	vec4 xPixel = pixel;
	vec4 yPixel = pixel;
	float gradientX = 0;
	float gradientY = 0;

	if (texCoord0.x > 1.0 - seamWidth) {
		seamCoord.x = 1.0 - texCoord0.x;  
		xPixel = texture(source,seamCoord);
		//xPixel = vec4(0,1,0,1);
		
		gradientX = (texCoord0.x - (1.0 - seamWidth)) / seamWidth;
		mergeX = smoothstep(0.0,0.70,gradientX);
		xPixel = mix(pixel,xPixel,mergeX);
		//xPixel = vec4(mergeX,0,0,1);
	}
	
	
	seamCoord = texCoord0;
	if (texCoord0.y > 1.0 - seamWidth) {
		seamCoord.y = 1.0 - texCoord0.y; 
		yPixel = texture(source,seamCoord);
		//yPixel = vec4(1,0,0,1);
		//float merge = pow(seamCoord.y * (1/seamWidth),falloff);
		//pixel = mix(pixelY,pixel,merge);
		
		
		gradientY = (texCoord0.y - (1.0 - seamWidth)) / seamWidth;
		mergeY = smoothstep(0.0,0.70,gradientY);
		yPixel = mix(pixel,yPixel,mergeY);
		//yPixel = vec4(0,0,mergeY,1);
	}
	

	
	//vec2 point = normalize(vec2(gradientX,gradientY));
	//float angle = dot(point,vec2(1,0));
	
	float ratio = (mergeX * 0.5) + 0.5 - (mergeY * 0.5);
	
	
	pixel = mix(yPixel,xPixel,ratio);
		
	

	FragColour = pixel;
	
};

