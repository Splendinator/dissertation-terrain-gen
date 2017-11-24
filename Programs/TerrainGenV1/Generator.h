#pragma once

struct point {
	float x, y;
};

class Generator
{

private:

	float m_minHeight, m_maxHeight, m_rockiness;

	// Function to linearly interpolate between a0 and a1
	// Weight w should be in the range [0.0, 1.0]
	float lerp(float a0, float a1, float w);

	// Computes the dot product of the distance and gradient vectors.
	float dotGridGradient(int ix, int iy, float x, float y);


	float Generator::fade(float t);

public:
	Generator();
	Generator(float minHeight, float maxHeight, float r);
	~Generator();


	

	// Compute Perlin noise at coordinates x, y
	float perlin(int x, int y);

	float simplex(int x, int y);
	
};

