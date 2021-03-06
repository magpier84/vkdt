#include "localsize.h"

// http://vec3.ca/bicubic-filtering-in-fewer-taps/
vec4 sample_catmull_rom(sampler2D tex, vec2 uv)
{
  // We're going to sample a a 4x4 grid of texels surrounding the target UV coordinate. We'll do this by rounding
  // down the sample location to get the exact center of our "starting" texel. The starting texel will be at
  // location [1, 1] in the grid, where [0, 0] is the top left corner.
  vec2 texSize = textureSize(tex, 0);
  vec2 samplePos = uv * texSize;
  vec2 texPos1 = floor(samplePos - 0.5) + 0.5;

  // Compute the fractional offset from our starting texel to our original sample location, which we'll
  // feed into the Catmull-Rom spline function to get our filter weights.
  vec2 f = samplePos - texPos1;

  // Compute the Catmull-Rom weights using the fractional offset that we calculated earlier.
  // These equations are pre-expanded based on our knowledge of where the texels will be located,
  // which lets us avoid having to evaluate a piece-wise function.
  vec2 w0 = f * ( -0.5 + f * (1.0 - 0.5*f));
  vec2 w1 = 1.0 + f * f * (-2.5 + 1.5*f);
  vec2 w2 = f * ( 0.5 + f * (2.0 - 1.5*f) );
  vec2 w3 = f * f * (-0.5 + 0.5 * f);

  // Work out weighting factors and sampling offsets that will let us use bilinear filtering to
  // simultaneously evaluate the middle 2 samples from the 4x4 grid.
  vec2 w12 = w1 + w2;
  vec2 offset12 = w2 / (w1 + w2);

  // Compute the final UV coordinates we'll use for sampling the texture
  vec2 texPos0 = texPos1 - vec2(1.0);
  vec2 texPos3 = texPos1 + vec2(2.0);
  vec2 texPos12 = texPos1 + offset12;

  texPos0 /= texSize;
  texPos3 /= texSize;
  texPos12 /= texSize;

  vec4 result = vec4(0.0);
  result += textureLod(tex, vec2(texPos0.x,  texPos0.y),  0) * w0.x * w0.y;
  result += textureLod(tex, vec2(texPos12.x, texPos0.y),  0) * w12.x * w0.y;
  result += textureLod(tex, vec2(texPos3.x,  texPos0.y),  0) * w3.x * w0.y;

  result += textureLod(tex, vec2(texPos0.x,  texPos12.y), 0) * w0.x * w12.y;
  result += textureLod(tex, vec2(texPos12.x, texPos12.y), 0) * w12.x * w12.y;
  result += textureLod(tex, vec2(texPos3.x,  texPos12.y), 0) * w3.x * w12.y;

  result += textureLod(tex, vec2(texPos0.x,  texPos3.y),  0) * w0.x * w3.y;
  result += textureLod(tex, vec2(texPos12.x, texPos3.y),  0) * w12.x * w3.y;
  result += textureLod(tex, vec2(texPos3.x,  texPos3.y),  0) * w3.x * w3.y;

  return result;
}

// cannibalised version of the above, softer kernel:
vec4 sample_soft(sampler2D tex, vec2 uv)
{
  vec2 texSize = textureSize(tex, 0);
  vec2 samplePos = uv * texSize;
  vec2 texPos1 = samplePos;

  vec2 texPos0 = texPos1 - vec2(1.5);
  vec2 texPos3 = texPos1 + vec2(1.5);
  vec2 texPos12 = texPos1;

  texPos0 /= texSize;
  texPos3 /= texSize;
  texPos12 /= texSize;

  vec4 result = vec4(0.0);
  result += textureLod(tex, vec2(texPos0.x,  texPos0.y),  0);
  result += textureLod(tex, vec2(texPos12.x, texPos0.y),  0);
  result += textureLod(tex, vec2(texPos3.x,  texPos0.y),  0);

  result += textureLod(tex, vec2(texPos0.x,  texPos12.y), 0);
  result += textureLod(tex, vec2(texPos12.x, texPos12.y), 0);
  result += textureLod(tex, vec2(texPos3.x,  texPos12.y), 0);

  result += textureLod(tex, vec2(texPos0.x,  texPos3.y),  0);
  result += textureLod(tex, vec2(texPos12.x, texPos3.y),  0);
  result += textureLod(tex, vec2(texPos3.x,  texPos3.y),  0);

  return result / 9.0f;
}

// use 2x2 lookups to simulate 3x3
vec4 sample_semisoft(sampler2D tex, vec2 uv)
{
  vec2 texSize = textureSize(tex, 0);
  vec2 texPosc = uv * texSize;

  vec2 texPos0 = texPosc - vec2(.5);
  vec2 texPos1 = texPosc + vec2(.5);

  texPos0 /= texSize;
  texPos1 /= texSize;

  vec4 result = vec4(0.0);
  result += textureLod(tex, vec2(texPos0.x, texPos0.y), 0);
  result += textureLod(tex, vec2(texPos1.x, texPos0.y), 0);
  result += textureLod(tex, vec2(texPos0.x, texPos1.y), 0);
  result += textureLod(tex, vec2(texPos1.x, texPos1.y), 0);

  return result / 4.0f;
}

// almost 5x5 support in 5 taps
vec4 sample_flower(sampler2D tex, vec2 uv)
{
  vec2 texSize = textureSize(tex, 0);
  vec2 tc = uv * texSize;

  vec2 off0 = vec2(1.2,  0.4);
  vec2 off1 = vec2(-0.4, 1.2);

  const float t = 36.0/256.0;
  vec4 result = textureLod(tex, uv, 0) * t;
  result += textureLod(tex, (tc + off0)/texSize, 0) * (1.0-t)/4.0;
  result += textureLod(tex, (tc - off0)/texSize, 0) * (1.0-t)/4.0;
  result += textureLod(tex, (tc + off1)/texSize, 0) * (1.0-t)/4.0;
  result += textureLod(tex, (tc - off1)/texSize, 0) * (1.0-t)/4.0;

  return result;
}

float luminance_rec2020(vec3 rec2020)
{
  // excerpt from the rec2020 to xyz matrix (y channel only)
  return dot(vec3(2.62700212e-01, 6.77998072e-01, 5.93017165e-02), rec2020);
}

// (c) christoph peters:
void evd2x2(
    out vec2 eval,
    out vec2 evec0,
    out vec2 evec1,
    mat2 M)
{
	// Define some short hands for the matrix entries
	float a = M[0][0];
	float b = M[1][0];
	float c = M[1][1];
	// Compute coefficients of the characteristic polynomial
	float pHalf = -0.5 * (a + c);
	float q = a*c - b*b;
	// Solve the quadratic
	float discriminant_root = sqrt(pHalf * pHalf - q);
	eval.x = -pHalf + discriminant_root;
	eval.y = -pHalf - discriminant_root;
	// Subtract a scaled identity matrix to obtain a rank one matrix
	float a0 = a - eval.x;
	float b0 = b;
	float c0 = c - eval.x;
	// The column space of this matrix is orthogonal to the first eigenvector 
	// and since the eigenvectors are orthogonal themselves, it agrees with the 
	// second eigenvector. Pick the longer column to avoid cancellation.
	float squaredLength0 = a0*a0 + b0*b0;
	float squaredLength1 = b0*b0 + c0*c0;
	float squaredLength;
	if (squaredLength0 > squaredLength1)
  {
		evec1.x = a0;
		evec1.y = b0;
		squaredLength = squaredLength0;
	}
	else {
		evec1.x = b0;
		evec1.y = c0;
		squaredLength = squaredLength1;
	}
	// If the eigenvector is exactly zero, both eigenvalues are the same and the 
	// choice of orthogonal eigenvectors is arbitrary
	evec1.x = (squaredLength == 0.0) ? 1.0 : evec1.x;
	squaredLength = (squaredLength == 0.0) ? 1.0 : squaredLength;
	// Now normalize
	float invLength = 1.0 / sqrt(squaredLength);
	evec1.x *= invLength;
	evec1.y *= invLength;
	// And rotate to get the other eigenvector
	evec0.x =  evec1.y;
	evec0.y = -evec1.x;
}

// returns true if the chromaticity coordinates given here
// lie outside an analytic approximation to the spectral locus
bool outside_spectral_locus(vec2 xy)
{
  if(xy.x + xy.y > 1) return true;
  if(xy.y < (xy.x-0.17)*0.47) return true;
  float f = 0.0509773/(xy.x+0.0563933) - 0.218247;
  if(xy.x < 0.18 && xy.y < f) return true;
  float g = 0.833 -3.83085*pow(xy.x-0.07,2.0) + 32.7412*pow(xy.x-0.07,4.0) -185.68*pow(xy.x-0.07,6.0);
  if(xy.x < 0.34 && xy.y > g) return true;
  float h = 0.833 -2.12221*pow(xy.x-0.07,2.0) -8601.78*pow(xy.x-0.07,4.0);
  if(xy.x < 0.07 && xy.y > h) return true;
  return false;
}
