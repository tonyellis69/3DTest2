#version 330

layout(location = 0) in vec3 points;

uniform vec3 pos;
uniform mat4 mvpMatrix;
uniform float lifeTime;
uniform float size;

out quad {
	vec4 a;
	vec4 b;
	vec4 c;
	vec4 d; 
} gQuad;

float w = 0.04;
float h = 0.04;

float baseSpeed = 1.0f;
float speedVariance = 8.0f;
float rotationVariance = 3.0f;
float baseRotationSpeed = 5.0f;

#define PI 3.1415926538
#define halfPI 1.5707963269


float rand(vec2 p){
    return fract(sin(dot(p.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 rand2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),
							dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 hash3( vec2 p ){
    vec3 q = vec3( dot(p,vec2(127.1,311.7)), 
				   dot(p,vec2(269.5,183.3)), 
				   dot(p,vec2(419.2,371.9)) );
	return fract(sin(q)*43758.5453);
}

mat4 rotationZ( in float angle ) {
	return mat4(	cos(angle),		-sin(angle),	0,	0,
			 		sin(angle),		cos(angle),		0,	0,
							0,				0,		1,	0,
							0,				0,		0,	1);
}


void main() {
	float r1 = rand(pos.xy); //always the same for this position
	vec3 randMove = hash3(vec2(r1,gl_VertexID)); //different for each particle
	vec3 randShape = hash3(vec2(gl_VertexID,r1));
	
	vec3 dirVec = vec3(randMove.xy - vec2(0.5),0);
	dirVec = normalize(dirVec);
	
	float speed;
	float rotation;
	
	float scale = mix(1.0f, 0.3f, 1 - size / 10.0f);
	float moveScale = mix(1.0f, 0.3f, 1 - size / 10.0f);
	
	float quadStart = 0.5f; 
	if (size < 5)
		quadStart = 1.0f;
	
	if (randShape.z < quadStart) { //it's a quad
		
		w *= scale; h *= scale;
		w += w * randShape.x * 0.7f ;
		h += h * randShape.y * 0.7f ;
	
		rotation = lifeTime * randMove.z  * rotationVariance;
		rotation += lifeTime * baseRotationSpeed;
		if (gl_VertexID % 2 == 0)
			rotation *= -1;
		speed = (baseSpeed * 0.5f * moveScale) + (speedVariance * randMove.z * 0.5f * moveScale);
	}
	else { //it's a line
		w = 0.01f;
		h = 0.75f * scale;
		
		rotation = atan(dirVec.x,dirVec.y) ;
		speed = (baseSpeed  * 4 * moveScale) + (speedVariance * randMove.z * moveScale );
		
	}
	
	vec3 velocity = dirVec * speed;
	vec4 disp = vec4(velocity * lifeTime + pos,0);
	
	mat4 rotMatrix = rotationZ(rotation);
	
	gQuad.a = disp + rotMatrix * vec4(w,h,0,1);
	gQuad.b = disp + rotMatrix * vec4(-w,h,0,1);
	gQuad.c = disp + rotMatrix * vec4(w,-h,0,1);
	gQuad.d = disp + rotMatrix * vec4(-w,-h,0,1);
	
	
	gQuad.a = mvpMatrix * gQuad.a;
	gQuad.b = mvpMatrix * gQuad.b;
	gQuad.c = mvpMatrix * gQuad.c;
	gQuad.d = mvpMatrix * gQuad.d;

};

