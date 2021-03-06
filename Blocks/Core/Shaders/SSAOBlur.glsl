// Original shader by mrharicot

#version 330 core

#define SIGMA 10.0
#define MSIZE 15

layout (location = 0) out float o_Color;

uniform sampler2D u_Texture;
uniform vec2 u_SketchSize;

float u_BSIGMA = 0.2f;

float normpdf(in float x, in float sigma)
{
	return 0.39894f * exp(-0.5f * x * x / (sigma * sigma)) / sigma;
}

float normpdf3(in vec3 v, in float sigma)
{
	return 0.39894f * exp(-0.5f * dot(v,v) / (sigma*sigma)) / sigma;
}

float GetLuminance(vec3 color) {
	return dot(color, vec3(0.299, 0.587, 0.114));
}

void main(void)
{
	float initial_col = texture(u_Texture, vec2(0.0, 0.0) + (gl_FragCoord.xy / u_SketchSize.xy)).r;
	float c = initial_col;
		
	const int kSize = 3;
	float final_colour;
	
	float Z = 0.0;

	// Precalculated kernet of size 15
	const float kernel[MSIZE] = float[MSIZE]
	(
			0.031225216, 0.033322271, 0.035206333, 
			0.036826804, 0.038138565, 0.039104044,
			0.039695028, 0.039894000, 0.039695028,
			0.039104044, 0.038138565, 0.036826804, 
			0.035206333, 0.033322271, 0.031225216
	);
	
	float cc;
	float factor;
	float bZ = 1.0 / normpdf(0.0, u_BSIGMA);

	//read out the texels
	for (int i = -kSize; i <= kSize; ++i)
	{
		for (int j = -kSize; j <= kSize; ++j)
		{
			cc = texture(u_Texture, vec2(0.0, 0.0) + ( gl_FragCoord.xy + vec2(float(i),float(j))) / u_SketchSize.xy).r;
			factor = normpdf(cc - c, u_BSIGMA) * bZ * kernel[kSize + j] * kernel[kSize + i];
			Z += factor;
			final_colour += factor * cc;

		}
	}
	
	o_Color = final_colour / Z;
}