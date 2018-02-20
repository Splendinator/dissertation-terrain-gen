#include <math.h>
#include "../../nclgl/Vector2.h"

enum Biome {
	FIELD, HILLS, DESERT, MAX_BIOMES
};

struct BiomePoint {
	Vector2 loc;
	Biome biome;
};

const int GRID_POINTS = 2;	//How many grid points around the player will be used for biome generation. Upping this will get better biome generation when combined with a higher offset but very slow.
const float BORDER_LERP = 130;

#pragma once
class BiomeMap
{
private:
	int m_gridSize;	//Average distance between each point
	int m_offset;		//Offset of each point

	BiomePoint randomPoint(int x, int y);

public:
	BiomeMap();

	BiomeMap(int gridSize, int offset);

	//Returns an array of biome percentages in the ENUM order.    Array length = MAX_BIOMES;
	float * getBiome(float x, float y);

	~BiomeMap();
};

