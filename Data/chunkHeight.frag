#version 400


layout(location = 0) out vec4 height;

smooth in vec3 vert;

void main() {
	
	//height = vec4(vert.y,vert.y,vert.y,1);
	height = vec4(gl_FragCoord.x/40,gl_FragCoord.y/40,0.0,1);
}