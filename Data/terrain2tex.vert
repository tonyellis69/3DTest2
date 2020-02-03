#version 330

layout(location = 0) in vec2 vertex; //One of four verts describing a screen-sized quad.


void main() {
	gl_Position = vec4(vertex,0,1);	


}
