#include "Generator.h"
#include <math.h>
#include <random>
#include <iostream>

Generator::Generator()
{

	
}

Generator::Generator(float minHeight, float maxHeight, float rockiness) {

	m_rockiness = rockiness;
	m_minHeight = minHeight;
	m_maxHeight = maxHeight;

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

//IX,IY - Co-oords of two points we are interpolating between.
float Generator::dotGridGradient(int ix, int iy, float x, float y) {


	// Compute the distance vector
	float dx = x - (float)ix;
	float dy = y - (float)iy;


	srand((ix* 104723 + iy * 104729));																//Generate heights for the points on the big grid.
	float min = m_minHeight + ((rand() % 100)*(m_maxHeight - m_minHeight)) / 100 + m_minHeight;		//Work out two points to interpolate between.
	float max = m_minHeight + ((rand() % 100)*(m_maxHeight - m_minHeight)) / 100 + m_minHeight;

	//std::cout << temp << " " << temp2 << std::endl;

	// Compute the dot-product
	return (dx * min + dy*max);
}

// Compute Perlin noise at coordinates x, y
float Generator::perlin(int x, int y) {
	x = abs(x);
	y = abs(y);
	//std::cout << x << " " << y << std::endl;

	float fx = x / m_rockiness;
	float fy = y / m_rockiness;

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
