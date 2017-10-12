#pragma once
#include "../../nclgl/HeightMap.h"

#define CHUNK_SIZE (RAW_HEIGHT - 1) * HEIGHTMAP_X

class Chunk
{
private:
	
	HeightMap *h;			//Heightmap that will be rendered on screen.
	Vector2 wc;				//X and Z world co-ordinates of chunk. 
	int prevId = 0;

public:
	Chunk *n = NULL, *e = NULL, *s = NULL, *w = NULL;	//Pointers to the chunk to the north,east,south,west.
	

	Chunk();
	Chunk(Vector2 pos);
	~Chunk();
	
	Vector2 getPosition() { return wc; };
	HeightMap *getHeightMap() { return h; };

	void Draw();

	void Generate();

	void makeHill(Vector2 pos, float dy, float rad, UINT id);

	void stitch(UINT sKey);

};

