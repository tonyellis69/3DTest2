#version 400

/**	This shader draws a top-down view of a given chunk, writing height values rather than colours. */

layout(location = 0) in vec3 vertex; //A vertex of this chunk's triangle mesh.

uniform mat4 topDownMatrix; //A matrix to create a top-down, orthographic view of this chunk.
smooth out vec3 vert;

void main() {
	gl_Position = topDownMatrix * vec4(vertex,1);	
	vert = vertex;
}
