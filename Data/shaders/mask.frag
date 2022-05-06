#version 330

layout(location = 0) out vec4 dummy;
layout(location = 1) out vec4 outputColor;

float near = 0.1; 
float far  = 1000.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main() {

	outputColor = vec4(1,0,0,1);
	//if (gl_FragCoord.z > 0.99)
	outputColor = vec4(0,gl_FragCoord.z,gl_FragCoord.z,0);
	//gl_FragDepth = 0;//gl_FragCoord.z;
	
	 float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
	
	//outputColor = vec4(depth,depth,depth,1);
}
