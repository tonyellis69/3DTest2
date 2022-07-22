#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in int colourId ;

uniform mat4 mvpMatrix;




void main() {
	gl_Position = mvpMatrix * (vec4(position,1)  /* * vec4(2,2,1,1) */ );


}