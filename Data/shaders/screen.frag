#version 330

in vec2 vTexCoord;
out vec4 colour;

uniform sampler2D screenBuf;
uniform sampler2D screenMask;


//const float offset = 1.0 / 600; //3000.0; 


void main() {
	
	 colour =  texture2D(screenBuf, vTexCoord.st );
	 vec4 mask =  texture2D(screenMask, vTexCoord.st );
	 
		//colour = mask;
	 
	 
	 colour = colour + mask;
	// colour = vec4(colour.rgb,1.0);

	
	vec3 rgb = mix(mask,colour,colour.a).rgb;
	float alpha = colour.a + ((1.0 - colour.a) * mask.a);
	
	//colour = vec4(rgb,alpha);
	

	//colour = mask;
};
