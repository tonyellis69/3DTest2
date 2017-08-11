#version 330

 layout(points) in;
 layout(triangle_strip, max_vertices = 12) out;
 
uniform mat4 mvpMatrix; //The standard model-to-perspective-view matrix.
uniform float time; //Current time in seconds.

 in vec4 position[];
 in mat4 passMatrix[];
 
smooth out vec2 texCoord;

vec3 vLocalSeed;

//Returns random number from zero to one
float randZeroOne()
{
    uint n = floatBitsToUint(vLocalSeed.y * 214013.0 + vLocalSeed.x * 2531011.0 + vLocalSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float fRes =  2.0 - uintBitsToFloat(n);
    vLocalSeed = vec3(vLocalSeed.x + 147158.0 * fRes, vLocalSeed.y*fRes  + 415161.0 * fRes, vLocalSeed.z + 324154.0*fRes);
    return fRes;
}

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

int randomInt(int min, int max)
{
	//float fRandomFloat = randZeroOne();
	float fRandomFloat = random(normalize(position[0].xz));
	return int(float(min)+fRandomFloat*float(max-min));
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}







 void main() {
	float PIover180 = 3.1415/180.0;
	vec3 vBaseDir[3];
	vBaseDir[0] = vec3(1.0, 0.0, 0.0);
	vBaseDir[1] = vec3(float(cos(45.0*PIover180)), 0.0f, float(sin(45.0*PIover180)));
	vBaseDir[2] =vec3(float(cos(-45.0*PIover180)), 0.0f, float(sin(-45.0*PIover180)));
	
	
	float fGrassPatchSize = 1.5;
	float fGrassPatchHeight = 0.5 + random(position[0].xz)*0.3; 
	
	float fWindStrength = 0.25;;
	vec3 vWindDirection = vec3(1.0, 0.0, 1.0);
	vWindDirection = normalize(vWindDirection);
 
 
	for (int i = 0; i < 3; i++) {
		//find the verts of the quad
		int iGrassPatch = randomInt(0, 3);
		float fTCStartX = float(iGrassPatch)*0.25f;
		float fTCEndX = fTCStartX+0.25f;
		
		float fWindPower = 0.5f+sin(position[0].x/5 + position[0].z/5 + time*(1.2f+fWindStrength/20.0f));
		if(fWindPower < 0.0f)
			fWindPower = fWindPower*0.2f;
		else fWindPower = fWindPower*0.3f;
		
		fWindPower *= fWindStrength;
		
		//bottom left vertex
		gl_Position = mvpMatrix * ( position[0] +    vec4(vBaseDir[i] * -fGrassPatchSize * 0.5,0));
		texCoord = vec2(fTCStartX,0);
		EmitVertex();
		
		//bottom right vertex
		gl_Position =  mvpMatrix * ( position[0] + vec4( vBaseDir[i] * fGrassPatchSize * 0.5,0));
		texCoord = vec2(fTCEndX,0);
		EmitVertex();
		
		//top left vertex
		gl_Position = mvpMatrix * ( position[0] + vec4((vBaseDir[i] * -fGrassPatchSize * 0.5) + vWindDirection*fWindPower,0) + vec4(0,fGrassPatchHeight,0,0));
		texCoord = vec2(fTCStartX,1);
		EmitVertex();
		
		//top right vertex
		gl_Position = mvpMatrix * (  position[0] + vec4((vBaseDir[i] * fGrassPatchSize * 0.5) + vWindDirection*fWindPower,0) + vec4(0,fGrassPatchHeight,0,0));
		texCoord = vec2(fTCEndX,1);
		EmitVertex();
		
		EndPrimitive();
	}
	
}
