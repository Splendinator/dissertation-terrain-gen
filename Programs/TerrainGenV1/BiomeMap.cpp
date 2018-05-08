#include "BiomeMap.h"



BiomeMap::BiomeMap()
{
}

BiomeMap::BiomeMap(int gridSize, int variance) {
	m_gridSize = gridSize;
	m_variance = variance;
}

unsigned char p[512] = { 151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

//Generates a random point given a grid location 
BiomePoint BiomeMap::randomPoint(int x, int y) {


	//srand((((((((((x * (y+1) * 7) & 480) >> 5) ^ 0b01101) + y * (x+1) * 19) & 480) >> 5) * ((x % 2047) + 1)) * ((y % 2047) + 1)) ^ 0b01101001000110110110110101110101 % RAND_MAX);

	unsigned char xr = p[(p[x & 255] + y) & 255];
	unsigned char yr = p[(xr + x + p[y&255])&255];

	BiomePoint b;
	b.loc = Vector2(x - m_variance + (float(xr)/128 * m_variance), y - m_variance + (float(yr) / 128 * m_variance));
	b.biome = Biome(p[(xr + yr)&255] & 0b11);
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

	dist *= dist;

	//Generate all grid points, and get distances for each point
	for (int i = 0; i < GRID_POINTS_WORLEY; i++) {
		for (int j = 0; j < GRID_POINTS_WORLEY; j++) {
			BiomePoint gridPoint = randomPoint(x - fmod(x, m_gridSize) + (m_gridSize * (i - (GRID_POINTS_WORLEY / 2 - 1))), y - fmod(y, m_gridSize) + (m_gridSize * (j - (GRID_POINTS_WORLEY / 2 - 1))));				//Generate the grid points.
			distance[i*GRID_POINTS_WORLEY + j] = (gridPoint.loc.x - x) * (gridPoint.loc.x - x) + (gridPoint.loc.y - y) * (gridPoint.loc.y - y);					//Calculate distances to each grid point using pythagerous.
			
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
