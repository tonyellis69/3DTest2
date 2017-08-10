#version 330

 layout(points) in;
 layout(triangle_strip, max_vertices = 4) out;

 in vec4 position[];
 in mat4 passMatrix[];
 //in vec3 dummyVec[];

 out vec4 corner; //The vertex we output for each corner of the quad
 smooth out vec4 fragColour;

 void main() {
	fragColour = vec4(0,1.0,0,1.0);
	
	
	float quadSize = 0.5;
 
	//find the  verts of the quad
	vec4 tmp =   position[0];
	
	gl_Position = passMatrix[0] * (tmp + vec4(-quadSize,0,0,0));
	//gl_Position = passMatrix[0] * tmp;
	//fragColour = vec4(dummyVec[0],1.0);
	EmitVertex();
	
	gl_Position =  passMatrix[0] * (tmp + vec4(quadSize,0,0,0));
	//gl_Position = passMatrix[0] * tmp;
	//fragColour = vec4(dummyVec[0],1.0);
	EmitVertex();
	
	gl_Position = passMatrix[0] * (tmp + vec4(-quadSize,quadSize,0,0));
	//gl_Position =  passMatrix[0] * tmp;
	//fragColour = vec4(dummyVec[0],1.0);
	EmitVertex();
	
	
	gl_Position = passMatrix[0] * ( tmp + vec4(quadSize,quadSize,0,0));
	//gl_Position = passMatrix[0] * tmp;
//	fragColour = vec4(dummyVec[0],1.0);
	EmitVertex();
	
	EndPrimitive();
	
}
