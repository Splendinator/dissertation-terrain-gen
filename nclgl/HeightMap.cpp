#include "HeightMap.h"

HeightMap::HeightMap(std::string name) {
	std::ifstream file(name.c_str(), ios::binary);
	if (!file) {
		return;
	}
	numVertices = RAW_WIDTH * RAW_HEIGHT;
	numIndices = (RAW_WIDTH - 1)*(RAW_HEIGHT - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];

	unsigned char * data = new unsigned char[numVertices];
	file.read((char *)data, numVertices * sizeof(unsigned char));
	file.close();

	for (int x = 0; x < RAW_WIDTH; ++x) {
		for (int z = 0; z < RAW_HEIGHT; ++z) {
			int offset = (x * RAW_WIDTH) + z;

			vertices[offset] = Vector3(
				x * HEIGHTMAP_X, data[offset] * HEIGHTMAP_Y, z * HEIGHTMAP_Z);

			textureCoords[offset] = Vector2(
				x * HEIGHTMAP_TEX_X, z * HEIGHTMAP_TEX_Z);
		}
	}

	delete data;
	numIndices = 0;

	for (int x = 0; x < RAW_WIDTH - 1; ++x) {
		for (int z = 0; z < RAW_HEIGHT - 1; ++z) {
			int a = (x * (RAW_WIDTH)) + z;
			int b = ((x + 1) * (RAW_WIDTH)) + z;
			int c = ((x + 1) * (RAW_WIDTH)) + (z + 1);
			int d = (x * (RAW_WIDTH)) + (z + 1);

			indices[numIndices++] = c;
			indices[numIndices++] = b;
			indices[numIndices++] = a;

			indices[numIndices++] = a;
			indices[numIndices++] = d;
			indices[numIndices++] = c;

			} 
	}
	
	BufferData();

}

void HeightMap::makeHill(Vector2 pos, float dy, float rad) {
	bool neg = false;
	int x, y,tx = RAW_WIDTH*pos.x,ty=RAW_WIDTH*pos.y;
	float dist;
	



	for (x = 0; x < RAW_WIDTH; x++) {
		for (y = 0; y < RAW_WIDTH; y++) {
			dist = sqrt((tx-x)*(tx-x) + (ty-y)*(ty-y));
			if (dist < rad) {
				//dist = dy*((rad - dist) / rad);
				vertices[x*RAW_WIDTH + y].y += sqrt(rad - dist)*dy; // y = sqrt(rad - x^2)		x=dy*((rad-dist)/rad)
			}
		}
	}
	BufferData();
}

void HeightMap::makeFlat() {
	for (int x = 0; x < RAW_WIDTH; x++) {
		for (int y = 0; y < RAW_WIDTH; y++) {
			vertices[x*RAW_WIDTH + y] = Vector3(vertices[x*RAW_WIDTH + y].x, 0.0f, vertices[x*RAW_WIDTH + y].z);
		}
	}
	BufferData();
}