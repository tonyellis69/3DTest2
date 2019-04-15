#version 330

in vec2 texCoord0;  

uniform sampler2D source; 
uniform int gridSize;
uniform bool showDetails = true;
uniform float cutOff = 0.02; 
uniform float broadCutoff = 0.1;//0.05


out vec4 FragColour;

float getTileValue(vec2 tile) {
	ivec2 p = ivec2(tile);
	return texelFetch(source,p,0).r;
	//vec2 p = (tile * 0.01) + vec2(0.001);
	//return texture(source,p).r;
}



void main() {  
	
	
	FragColour = vec4(0,0,0,1);
	
	float cellValue = texture(source,texCoord0).r;
	
	if (showDetails) {
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
	}
	else {
		float minDistance = 1;
		float fineMask = cellValue;
		float broadMask = cellValue;
		for (float x = texCoord0.x - cutOff; x < texCoord0.x + cutOff; x+=0.001) {
			for (float y = texCoord0.y - cutOff; y < texCoord0.y + cutOff; y+=0.001) {
				if (texture(source,vec2(x,y)).r == 1) {
					float dist = distance(texCoord0, vec2(x,y)) ;
					minDistance = min(dist,minDistance);
				}
			}
		}
		
		if (minDistance < 1)
			fineMask = 1 - (minDistance / cutOff) ;
		
		minDistance = 1;
		for (float x = texCoord0.x - broadCutoff; x < texCoord0.x + broadCutoff; x+=0.001) {
			for (float y = texCoord0.y - broadCutoff; y < texCoord0.y + broadCutoff; y+=0.001) {
				if (texture(source,vec2(x,y)).r == 1) {
					float dist = distance(texCoord0, vec2(x,y)) ;
					minDistance = min(dist,minDistance);
				}
			}
		}
		
		if (minDistance < 1)
			broadMask = (1 - (minDistance / broadCutoff)) ;//* 0.5 ;
		
		
		vec3 mask = vec3(fineMask,broadMask,0);
		/*float margin = 0.001;// 1 / gridSize;
		mask = mask * step(margin,texCoord0.x) * step(margin,texCoord0.y) 
				* (1- step(1-margin,texCoord0.x)) * (1 -step(1-margin,texCoord0.y)); 
		*/
		
		//if (texCoord0.x > 0.8 && texCoord0.y > 0.8 && texCoord0.x < 0.99 && texCoord0.y < 0.99 )
		//	mask = vec3(1);
	
		float endPointDist = distance(texCoord0,vec2(0.8));
		float endArea = 1 - smoothstep(0.1,0.2,endPointDist);
		mask.g = max(endArea,mask.g);
		
		
		FragColour = vec4(mask,1);
		
		
		
		
	}
	
	vec2 point = texCoord0 * gridSize;
	point = fract(point);
	
	if (showDetails)
	FragColour += (step(0.98, point.x) + step(0.98, point.y));	
	
}