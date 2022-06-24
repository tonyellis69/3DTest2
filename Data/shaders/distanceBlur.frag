#version 330

in vec2 vTexCoord;
out vec4 colour;

uniform sampler2D srcTexture;

uniform bool horizontal;
uniform int blurSize = 20;   


vec2 texOffset = 1.0 / textureSize(srcTexture, 0);//vec2(0.00195);

float distFn(vec2 p) {
	return abs(p.x) + abs(p.y);
}

void main() {
	vec2 p = vTexCoord;
	
	
	colour = texture2D(srcTexture, p);
	
	if (length(colour) > 0) {
		//colour = vec4(1,0,0,1);
		return;
	}
	
	
	vec4 curValue = vec4(0);
	float curDist = 9999;
	int extent = blurSize/2;
	
	for (int j = -extent; j <= extent; j++) {
		for (int i = -extent; i <= extent; i++) {
			float dist = distFn(vec2(i,j));
			
			if (dist < curDist && dist > 0) {
				vec4 neighb = texture(srcTexture, vTexCoord + vec2(texOffset.x * i, texOffset.y * j));
				if ( length(neighb) > 0) {
				
					curDist = dist;
					curValue = neighb;
				}
				
				
			}
        }
	}
	
	if (curDist < 9999) {
		float fade = 1 - (curDist / distFn(vec2(extent)));
		colour = curValue * fade;
		//colour = vec4(colour.rgb,1);
		
	}
	
	

   
};
