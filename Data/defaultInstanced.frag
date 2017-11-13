#version 440
//#include noise.lib

smooth in vec4 worldPos; 
smooth in vec4 fragColour;

out vec4 outputColor;

void main()
{
	outputColor = fragColour;
	
	
	float v;
	//v = octave_noise_2d(6,   1.5,   0.5,  0.5,   worldPos.xz/20);
	//v *= 0.5f; v += 0.5f;
	
	//outputColor = mix(fragColour,vec4(v,v,v,1),v);
	//outputColor.w = 1;
	
	
}
