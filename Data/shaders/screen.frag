#version 330

in vec2 vTexCoord;
out vec4 colour;

uniform sampler2D screenBuf;
uniform sampler2D screenMask;
uniform sampler2D blurTex;


uniform int x = 0;


void main() {
	
	 colour =  texture2D(screenBuf, vTexCoord.st );
	 vec4 mask =  texture2D(screenMask, vTexCoord.st );
	 vec4 blur = texture2D(blurTex, vTexCoord.st );
	 
	 float blurStrength = length(blur.rgb);
	 blurStrength = blur.r;
	 blurStrength *= 1.1;
	 
	 //blurStrength = clamp(blurStrength,0,0.5);
	//blurStrength *= .3;
	//blurStrength = blur.r;
	//blurStrength *= 1.5;
	
	if (x == 0)	{
		blurStrength = pow(blurStrength,1.5);
		 blur = vec4(blurStrength,blurStrength,blurStrength,1);
		 mask.a *= 1.25;
		colour = vec4(colour.rgb * mask.a,1);
		colour = mix(blur,colour,mask.a ) ;
		
	}
	else {
		//colour = vec4(colour.rgb * mask.a,1);
		
		 blur = vec4(blurStrength,blurStrength,blurStrength,1);
		 mask.a *= 1.25;
		colour = vec4(colour.rgb * mask.a,1);
		colour = mix(blur,colour,mask.a ) ;
	}
	 
	 
	//colour =  vec4(colour.rgb + mask.rgb,1);
	// colour = vec4(colour.rgb,1.0);

	
	vec3 rgb = mix(mask,colour,colour.a).rgb;
	float alpha = colour.a + ((1.0 - colour.a) * mask.a);
	
	//colour = vec4(rgb,alpha);
	

	//colour = mask;
	
	
};
