#version 330

uniform sampler2D grassTex; //Grass texture

smooth in vec2 texCoord;

out vec4 outputColor;

float fAlphaMultiplier = 2.0;  //1.50;
float fAlphaTest = 0.75; //0.25;

void main() {
	//outputColor = texture(grassTex,texCoord);
	
	
	
	
	
	
	vec4 vTexColor = texture2D(grassTex, texCoord); 
   float fNewAlpha = vTexColor.a*fAlphaMultiplier;                
   if(fNewAlpha < fAlphaTest) 
      discard; 
    
   if(fNewAlpha > 1.0f) 
      fNewAlpha = 1.0f;    
       
   vec4 vMixedColor = vTexColor; ///*vColor;  
    
   outputColor = vec4(vMixedColor.zyx, fNewAlpha);
	
		
}
