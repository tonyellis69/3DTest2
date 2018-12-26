#version 330

#include noise.lib

in vec2 texCoord0;
in vec2 samplePoint0;

out vec4 FragColor;

uniform int octaves;
uniform float frequency;

in vec3 samplePos0;
in vec3 sampleSize0;


void main() {
	
	float noise =  fbm2DclassicFreq(octaves,samplePoint0, frequency);
	float xSeam = noise;
	float ySeam = noise;
	vec2 mixFactor = vec2(0); 

	
	
	
	if (texCoord0.x > seamSize) {
		sampleFinal.x =   samplePoint0.x -
		sampleSize0.x;
		xSeam =  fbm2DclassicFreq(octaves,sampleFinal, frequency);
		//noise = mix(noise,seamNoise,mixFactor.x);
		mixFactor.x = ((texCoord0.x) -seamSize) * scale; //0 to 1
	}
	
	if (texCoord0.y > seamSize) {
		sampleFinal = samplePoint0;
		sampleFinal.y =   samplePoint0.y -
		sampleSize0.y;
		ySeam =  fbm2DclassicFreq(octaves,sampleFinal, frequency);
		//noise = mix(noise,seamNoise,mixFactor.y);
		mixFactor.y = ((texCoord0.y)- seamSize) * scale;
	}
	
	//merge the x and y seam based on their extent
	//probably x-y + 0.5
	float finalSeamMix = ((mixFactor.x - mixFactor.y) * 0.5) + 0.5f;
	float finalSeam = mix(ySeam,xSeam,finalSeamMix);
	
	//then try mixing with noise using whichever extent is larger
	float noiseSeamMix = max(mixFactor.x,mixFactor.y);
	noise = mix(finalSeam,noise,noiseSeamMix);
	

	noise = noise * 0.5 + 0.5;
	FragColor = vec4(noise,noise,noise,1 );
	
};