#version 330

layout(location = 0) in ivec2 index;
in uint content;

uniform mat4 mvpMatrix;

uniform ivec2 gridSize; 

uniform samplerBuffer effectsTex;

out hexQuad { //describes a quad that describes a hex
	vec4 a;
	vec4 b;
	vec4 c;
	vec4 d;
	
	float fog;
	float visibility;
	float highlight;

	vec3[6] neighbours;
	
	ivec3 cube;
	ivec2 index;
		
} hexQuadOut;


const float quadExtent = 1.0f;
const float hexWidth = sqrt(3.0f);
const float hexHeight = 2;


ivec3[] moveVectorCube = ivec3[] (ivec3(1,-1,0), ivec3(0,-1,1), ivec3(-1,0,1), ivec3(-1,1,0), ivec3(0,1,-1), ivec3(1,0,-1) );

ivec3[] latticeOffsets = ivec3[] ( 	ivec3(0,1,-1), ivec3(1,0,-1), ivec3(2,-1,-1), 
									ivec3(-1,1,0), ivec3(0,0,0), ivec3(1,-1,0), 
									ivec3(-2,1,1), ivec3(-1,0,1), ivec3(0,-1,1) );


/** Convert odd-row offset coordinates to cube coordinates. */
ivec3 offsetToCube(ivec2 index) {
	index -= gridSize / 2;
	
	ivec3 hex;
	hex.x = index.x - (index.y - (index.y & 1)) / 2;
	hex.z = index.y;		
	hex.y = -hex.x - hex.z;
	return hex;
}

/** Convert cube coordinates to odd-row offset coordinates.*/
ivec2 cubeToOffset(ivec3 hex) {
	ivec2 offset = ivec2(hex.x + (hex.z - (hex.z & 1)) / 2, hex.z);
	
	offset += gridSize/2;
	
	return offset;
}


/** Return the adjacent hex in this direction. */
ivec3 getNeighbour(ivec3 hex, int direction){
	return hex + moveVectorCube[direction];
}

vec4 getTexel(int latticePos, ivec3 centreHex) {
	ivec3 hex = centreHex + latticeOffsets[latticePos];
	ivec2 hexIndex = cubeToOffset(hex);
	int flatIndex = (hexIndex.y * gridSize.x) + hexIndex.x;
	return texelFetch(effectsTex,flatIndex);
}

void main() {

	ivec2 centreOffset = index - ivec2(gridSize * 0.5f);

	vec3 hexPosition = vec3( (centreOffset.x + (0.5f * (centreOffset.y & 1))) * hexWidth,
		-centreOffset.y * 1.5f,0);
		
		
	hexQuadOut.a = mvpMatrix * ( vec4(hexPosition,1) +  vec4(quadExtent,quadExtent,0,0) );
	hexQuadOut.b = mvpMatrix * ( vec4(hexPosition,1) +  vec4(-quadExtent,quadExtent,0,0) );
	hexQuadOut.c = mvpMatrix * ( vec4(hexPosition,1) +  vec4(quadExtent,-quadExtent,0,0) );
	hexQuadOut.d = mvpMatrix * ( vec4(hexPosition,1) +  vec4(-quadExtent,-quadExtent,0,0) );
	

	int fogIndex = (index.y * gridSize.x) + index.x;
	vec3 visData =  texelFetch(effectsTex, fogIndex).rgb;
	hexQuadOut.fog = visData.r;
	hexQuadOut.visibility = visData.g;
	hexQuadOut.highlight = visData.b;

	
	ivec3 hex = offsetToCube(index);
	for (int n = 0; n < 6; n++) {
		ivec3 neighbourHex =  getNeighbour(hex,n);
		ivec2 offset = cubeToOffset(neighbourHex);
		int flatIndex = (offset.y * gridSize.x) + offset.x;	
		hexQuadOut.neighbours[n] = texelFetch(effectsTex,flatIndex).rgb;
	}
	
	
	
	hexQuadOut.cube = hex;
	hexQuadOut.index = index;
}



