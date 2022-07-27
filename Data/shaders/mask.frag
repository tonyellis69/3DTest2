#version 330

layout(location = 0) out vec4 dummy;
layout(location = 1) out vec4 dummy2; //CHECK: needed?
layout(location = 2) out vec4 outputColor;


void main() {

	outputColor = vec4(1,0,1,0);

}
