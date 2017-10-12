#pragma once

#include <string>
#include <iostream>
#include <fstream>

#include "..\..\nclgl\mesh.h"

#define RAW_WIDTH 257
#define RAW_HEIGHT 257

#define HEIGHTMAP_X 8.0f
#define HEIGHTMAP_Z 8.0f
#define HEIGHTMAP_Y 0.8f
#define HEIGHTMAP_TEX_X 1.0f / 16.0f
#define HEIGHTMAP_TEX_Z 1.0f / 16.0f

class HeightMap : public Mesh {
public:
	HeightMap(Vector2 pos);
	~HeightMap(void) {};
	//void makeHill(Vector2 pos, float dy, float rad);
	void makeFlat();
};