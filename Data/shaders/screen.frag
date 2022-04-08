#version 330

in vec2 vTexCoord;
out vec4 colour;

uniform sampler2D screenBuf;
uniform sampler2D screenMask;


//const float offset = 1.0 / 600; //3000.0; 

float offset = 1.0 / 1000;

void main() {
	
	 colour =  texture2D(screenBuf, vTexCoord.st );
	   
	
	vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

	
	float kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
	);
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture2D(screenMask, vTexCoord.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
    
	col *= 0.75f;
   colour = colour + vec4(col, 1.0);
   
  // colour = vec4(col, 1.0);

   
   
   
};
