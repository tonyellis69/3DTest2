#version 330

uniform vec4 colour = vec4(0,1,0,1);

in vec2 lineCentre;

out vec4 outputColor;

const float blendFactor = 1.5f;

void main() {
	outputColor = colour;
	
	       
      float d = length(lineCentre - gl_FragCoord.xy);
      float w = 1.0f; //uLineWidth;
	  
      if (d>w)
        outputColor = vec4(1,0,0,0);
      else
        outputColor.w *= pow(float((w-d)/w), blendFactor);

}
