#include "Generator.h"
#include <math.h>
#include <random>
#include <iostream>

Generator::Generator()
{

	//std::random_device(e);
	//std::uniform_real_distribution<float> rand(-300.0f, 300.0f);
	//rockiness = 50.0f;
	//for (int i = 0; i < IYMAX; i++) {
	//	for (int j = 0; j < IXMAX; j++) {
	//		*Gradient[i][j][0] = rand(e);
	//		*Gradient[i][j][1] = rand(e);
	//	}
	//}
	
}

Generator::Generator(float minHeight, float maxHeight, float r) {

	std::random_device(e);
	std::uniform_real_distribution<float> rand(minHeight, maxHeight);
	rockiness = r;
	Gradient = new float[IYMAX*IXMAX*2];

	for (int i = 0; i < IYMAX; i++) {
		for (int j = 0; j < IXMAX; j++) {
			Gradient[2 * (i*IYMAX + j) + 0] = rand(e);
			Gradient[2 * (i*IYMAX + j) + 1] = rand(e);
		}
	}
}


Generator::~Generator()
{
}

// Function to linearly interpolate between a0 and a1
// Weight w should be in the range [0.0, 1.0]
float Generator::lerp(float a0, float a1, float w) {
	return (1.0 - w)*a0 + w*a1;
}

// Computes the dot product of the distance and gradient vectors.
float Generator::dotGridGradient(int ix, int iy, float x, float y) {

	// Compute the distance vector
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	// Compute the dot-product
	return (dx*Gradient[2 * (iy*IYMAX + ix) + 0] + dy*Gradient[2 * (iy*IYMAX + ix) + 1]);
}

// Compute Perlin noise at coordinates x, y
float Generator::perlin(int x, int y) {
	float fx = ((IXMAX / 2)*rockiness + x) / rockiness;
	float fy = ((IYMAX / 2)*rockiness + y) / rockiness;

	// Determine grid cell coordinates
	int x0 = fx;
	int x1 = x0 + 1;
	int y0 = fy;
	int y1 = y0 + 1;

	// Determine interpolation weights
	// Could also use higher order polynomial/s-curve here
	float sx = fx - (float)x0;
	float sy = fy - (float)y0;

	// Interpolate between grid point gradients
	float n0, n1, ix0, ix1, value;
	n0 = dotGridGradient(x0, y0, fx, fy);
	n1 = dotGridGradient(x1, y0, fx, fy);
	ix0 = lerp(n0, n1, sx);
	n0 = dotGridGradient(x0, y1, fx, fy);
	n1 = dotGridGradient(x1, y1, fx, fy);
	ix1 = lerp(n0, n1, sx);
	value = lerp(ix0, ix1, sy);

 	return value;
}
