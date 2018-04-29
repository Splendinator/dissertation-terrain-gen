#include "BiomeMap.h"



BiomeMap::BiomeMap()
{
}

BiomeMap::BiomeMap(int gridSize, int variance) {
	m_gridSize = gridSize;
	m_variance = variance;
}

//Generates a random point given a grid location 
BiomePoint BiomeMap::randomPoint(int x, int y) {
	
	
	srand((((((((((x * (y+1) * 7) & 480) >> 5) ^ 0b01101) + y * (x+1) * 19) & 480) >> 5) * ((x % 2047) + 1)) * ((y % 2047) + 1)) ^ 0b01101001000110110110110101110101 % RAND_MAX);
	
	BiomePoint b;
	b.loc = Vector2(x - m_variance + (rand() % (2 * m_variance)), y - m_variance + (rand() % (2 * m_variance)));
	b.biome = Biome(rand() % MAX_BIOMES);
	return b;
}

void BiomeMap::getBiome(float x, float y, float *results) {

	x = abs(x);
	y = abs(y);

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

}

float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }	// 6t5 - 15t4 + 10t3

float BiomeMap::getWorley(float x, float y, float minHeight, float maxHeight, float dist, int n) {

	x = abs(x);
	y = abs(y);

	float distance[GRID_POINTS_WORLEY * GRID_POINTS_WORLEY]; //Distance to each point.


	//Generate all grid points, and get distances for each point
	for (int i = 0; i < GRID_POINTS_WORLEY; i++) {
		for (int j = 0; j < GRID_POINTS_WORLEY; j++) {
			BiomePoint gridPoint = randomPoint(x - fmod(x, m_gridSize) + (m_gridSize * (i - (GRID_POINTS_WORLEY / 2 - 1))), y - fmod(y, m_gridSize) + (m_gridSize * (j - (GRID_POINTS_WORLEY / 2 - 1))));				//Generate the grid points.
			distance[i*GRID_POINTS_WORLEY + j] = sqrt((gridPoint.loc.x - x) * (gridPoint.loc.x - x) + (gridPoint.loc.y - y) * (gridPoint.loc.y - y));					//Calculate distances to each grid point using pythagerous.
			
		}
	}
	


	//Find N'th closest point point.
	float *value = new float[n+1];
	value[0] = 0;
	for (int j = 1; j < n + 1; ++j) {
		value[j] = dist;
	}

	for (int i = 0; i < (GRID_POINTS_WORLEY * GRID_POINTS_WORLEY); ++i) {
		//std::cout << distance[i] << " " << std::endl;
		for (int j = 1; j < n+1; ++j) {
			if (distance[i] <= value[j] && distance[i] >= value[j - 1]) {
				for (int k = n; k > j; --k) {
					value[k] = value[k - 1];
				}
				value[j] = distance[i];
				break;
			}
		}
	}
	//std::cout << "P: " << value[0] << " " << value[1] << " " << value[2] << std::endl << std::endl;
	//float height = fade(value[n]/dist)
	float height = minHeight + fade((value[n] / dist) * 2 - 0)/3 * (maxHeight - minHeight) ;
	delete[] value;

	return height;
	
}

BiomeMap::~BiomeMap()
{
}
