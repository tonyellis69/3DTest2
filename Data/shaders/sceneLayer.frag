#version 330

in vec2 vTexCoord;
out vec4 colour;

uniform sampler2D mapTexture;
uniform sampler2D modelsTexture;
uniform sampler2D blurTexture;



void main() {
	
	vec4 mapColour =  texture2D(mapTexture, vTexCoord.st );
	vec4 modelsColour =  texture2D(modelsTexture, vTexCoord.st );
	vec4 blurColour = texture2D(blurTexture, vTexCoord.st );
	
	vec3 blurBright = blurColour.rgb * 1.5;
	blurBright = min(blurBright,0.4);

	vec3 mapAndBlur =  mapColour.rgb;// 
	
	//vec3 mapAndBlur = mapColour.rgb * (1 - blurColour.a) + (blurBright );
	
	vec3 final = modelsColour.rgb /* vec3(1) * modelsColour.a*/  + mapAndBlur.rgb * (1-modelsColour.a);


	colour = vec4(final,1);
	
	
	
	colour = blurColour;
	//colour.a = 0.25f;
	//colour.r = 0;
	//colour.g = 0;
	

};
