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

#define NUM_TEXTURES 2

class HeightMap : public Mesh {
public:
	GLuint textureSnow;
	GLuint textureGrass;
	GLuint textureSand;
	GLuint textureRock;
	GLuint textureWater;

	GLuint textureBufferObject;
	GLuint shadeBufferObject;
	GLuint waterBufferObject;
	
	

	HeightMap(Vector2 pos);
	HeightMap();
	~HeightMap(void) { 
		delete[] texturePct;
		delete[] shadePct;
		delete[] water;
	};

	void operator=(HeightMap &rhs);

	//void makeHill(Vector2 pos, float dy, float rad);
	void makeFlat();
	void Draw(); //Overloaded from Mesh.h to support multiple textures.

	void BufferData();

	Vector4 *texturePct;
	float *shadePct;
	float *water;
};