#version 330

in vec2 vTexCoord;
out vec4 colour;

uniform sampler2D srcTexture;

uniform bool horizontal;
float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);


void main() {
		
	vec2 tex_offset = 1.0 / textureSize(srcTexture, 0); // gets size of single texel
    vec3 result = texture(srcTexture, vTexCoord).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(srcTexture, vTexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(srcTexture, vTexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(srcTexture, vTexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(srcTexture, vTexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    colour = vec4(result, 1.0);
   
};
