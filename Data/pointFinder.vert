#version 400

layout(location = 0) in vec2 pointPos; //An xz point somewhere in the given heightfield.

uniform sampler2D heightField; //A heightfield texture.

out vec3 point; //the xzy point we output.



void main () {
	point.xz = pointPos;
	
	
	vec2 fPoint = pointPos/40;;
	point.y = texture(heightField, pointPos,0).x;
	
	//point.y = texelFetch(heightField, ivec2(pointPos),0).y;
	
	//point = gather.xyz;
	
}