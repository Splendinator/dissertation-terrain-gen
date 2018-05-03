#include <math.h>
#include <algorithm>
#include "../../nclgl/Vector2.h"

enum Biome {
	FIELD, HILLS, DESERT, ISLAND, MAX_BIOMES
};

struct BiomePoint {
	Vector2 loc;
	Biome biome;
};

const static int GRID_POINTS = 2;	//How many grid points around the player will be used for biome generation. Upping this will get better biome generation when combined with a higher variance but very slow.
const static int GRID_POINTS_WORLEY = 2;
const float BORDER_LERP = 400;

#pragma once
class BiomeMap
{
private:
	int m_gridSize;	//Average distance between each point
	int m_variance;		//variance of each point

	BiomePoint randomPoint(int x, int y);

public:
	BiomeMap();

	BiomeMap(int gridSize, int variance);

	//Returns an array of biome percentages in the ENUM order.    Array length = MAX_BIOMES;
	void getBiome(float x, float y, float *results);

	//Get worley noise height level.
	float getWorley(float x, float y, float minHeight, float maxHeight, float dist, int n);

	~BiomeMap();
};

