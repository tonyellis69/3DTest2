#version 330

in vec2 vTexCoord;
out vec4 colour;

uniform sampler2D srcTexture;

uniform bool horizontal;
uniform int blurSize = 7;   

uniform float sigma = 2;        // The sigma value for the gaussian function: higher value means more blur
                            // A good value for 9x9 is around 3 to 5
                            // A good value for 7x7 is around 2.5 to 4
                            // A good value for 5x5 is around 2 to 3.5
                            // ... play around with this based on what you need :)
const float PI = 3.14159265;
vec2 texOffset = 1.0 / textureSize(srcTexture, 0);//vec2(0.00195);

void main() {
		
 vec2 p = vTexCoord;
  float numBlurPixelsPerSide = float(blurSize / 2); 
  
  vec2 blurMultiplyVec =  horizontal ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  
  // Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
  vec3 incrementalGaussian;
  incrementalGaussian.x = 1.0 / (sqrt(2.0 * PI) * sigma);
  incrementalGaussian.y = exp(-0.5 / (sigma * sigma));
  incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;

  vec4 avgValue = vec4(0.0, 0.0, 0.0, 0.0);
  float coefficientSum = 0.0;

  // Take the central sample first...
  avgValue += texture2D(srcTexture, p) * incrementalGaussian.x;
  coefficientSum += incrementalGaussian.x;
  incrementalGaussian.xy *= incrementalGaussian.yz;

  // Go through the remaining 8 vertical samples (4 on each side of the center)
  for (float i = 1.0; i <= numBlurPixelsPerSide; i++) { 
    avgValue += texture2D(srcTexture, p - i * texOffset * blurMultiplyVec) * incrementalGaussian.x;         
    avgValue += texture2D(srcTexture, p + i * texOffset * blurMultiplyVec) * incrementalGaussian.x;         
    coefficientSum += 2.0 * incrementalGaussian.x;
    incrementalGaussian.xy *= incrementalGaussian.yz;
  }

  colour = avgValue / coefficientSum;
   
};
