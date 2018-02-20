#include "BiomeMap.h"



BiomeMap::BiomeMap()
{
}

BiomeMap::BiomeMap(int gridSize, int offset) {
	m_gridSize = gridSize;
	m_offset = offset;
}

//Generates a random point given a grid location 
BiomePoint BiomeMap::randomPoint(int x, int y) {
	
	srand((x * 104723 + y * 104729));
	
	BiomePoint b;
	b.loc = Vector2(x - m_offset + (rand() % (2 * m_offset)), y - m_offset + (rand() % (2 * m_offset)));
	b.biome = Biome(rand() % MAX_BIOMES);
	return b;
}

float * BiomeMap::getBiome(float x, float y) {

	x = abs(x);
	y = abs(y);

	float results[MAX_BIOMES];	//Returning this; array of percentages of each biome that influences this point.

	float distance[GRID_POINTS * GRID_POINTS]; //Distance to each point.
	Biome biome[GRID_POINTS * GRID_POINTS];	//Biome of each point.
	
	

	//Generate all grid points, and get distances for each point
	for (int i = 0; i < GRID_POINTS; i++) {
		for (int j = 0; j < GRID_POINTS; j++) {
			BiomePoint gridPoint = randomPoint(x - fmod(x, m_gridSize) + (m_gridSize * (i - (GRID_POINTS/2 - 1))), y - fmod(y, m_gridSize) + (m_gridSize * (j - (GRID_POINTS / 2 - 1))));				//Generate the grid points.
			distance[i*GRID_POINTS + j] = sqrt((gridPoint.loc.x - x) * (gridPoint.loc.x - x) + (gridPoint.loc.y - y) * (gridPoint.loc.y - y));					//Calculate distances to each grid point using pythagerous.
			biome[i*GRID_POINTS + j] = gridPoint.biome;
		}
	}


	//Find closest point.
	float value = INFINITY;
	int index = 0;

	for (int i = 0; i < GRID_POINTS * GRID_POINTS; i++) {
		if (distance[i] < value) {
			value = distance[i];
			index = i;
		}
	}


	//Working out percentage influences of all biomes.
	float totalDistance = 0;
	
	for (int i = 0; i < GRID_POINTS * GRID_POINTS; i++) {
		if (distance[i] < (value + BORDER_LERP)) {
			distance[i] = BORDER_LERP - (distance[i] - value);
			totalDistance += distance[i];
		}
		else {
			distance[i] = 0;
		}
	}

	//Returning Results
	for (int i = 0; i < MAX_BIOMES; i++) {
		results[i] = 0;	//Initialise to 0.
	}
	for (int i = 0; i < GRID_POINTS * GRID_POINTS; i++) {
		results[biome[i]] += distance[i] / totalDistance;
	}


	return results;
}

BiomeMap::~BiomeMap()
{
}
