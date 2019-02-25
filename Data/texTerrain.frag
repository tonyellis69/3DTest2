#version 330

in vec2 texCoord0;  

uniform sampler2D source; 
uniform int gridSize;
uniform bool showDetails = true;

out vec4 FragColour;

float getTileValue(vec2 tile) {
	ivec2 p = ivec2(tile);
	return texelFetch(source,p,0).r;
	//vec2 p = (tile * 0.01) + vec2(0.001);
	//return texture(source,p).r;
}

float smoothTile(vec2 tile) {
	return getTileValue(tile);
	float sum = 0;
	sum += getTileValue(tile + vec2(0,1.0));
	sum += getTileValue(tile + vec2(1.0,1.0));
	sum += getTileValue(tile + vec2(1.0,0));
	sum += getTileValue(tile + vec2(1.0,-1.0));
	sum += getTileValue(tile + vec2(0,-1.0));
	sum += getTileValue(tile + vec2(-1.0,-1.0));
	sum += getTileValue(tile + vec2(-1.0,0));
	sum += getTileValue(tile + vec2(-1.0,1.0));
	sum += getTileValue(tile + vec2(0,0));
	return sum / 9.0;
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
		
		float tileSize = 1.0 / gridSize;
	
		//what tile are we in?
		vec2 tile = floor(texCoord0 * gridSize) ;
		//what offset within tile?
		//vec2 offset = fract(texCoord0 * gridSize);
		vec2 offset = smoothstep(0,1,fract(texCoord0 * gridSize));
		
		float q11 = smoothTile(tile + ivec2(0));
		float q12 = smoothTile(tile + ivec2(0,1));
		float q21 = smoothTile(tile + ivec2(1,0));
		float q22 = smoothTile(tile + ivec2(1,1));
		
		float r1 = mix(q11,q21,offset.x);
		float r2 = mix(q12,q22,offset.x);
		
		float tileShade = mix(r1,r2,offset.y);
		
		//tileShade += cellValue;
	
		FragColour = vec4(vec3(tileShade),1);
	}
	
	vec2 point = texCoord0 * gridSize;
	point = fract(point);
	
	if (showDetails)
	FragColour += (step(0.98, point.x) + step(0.98, point.y));	
	
}