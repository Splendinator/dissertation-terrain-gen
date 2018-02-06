#pragma once
#include "../../nclgl/HeightMap.h"

#define CHUNK_SIZE (RAW_HEIGHT - 1) * HEIGHTMAP_X

enum Direction {
	NORTH, EAST, SOUTH, WEST
};

class Chunk
{
private:

	int prevId = 0;

public:

	Vector2 wc;				//X and Z world co-ordinates of chunk. 

	HeightMap *h;			//Heightmap that will be rendered on screen.
	Chunk *n = NULL, *e = NULL, *s = NULL, *w = NULL;	//Pointers to the chunk to the north,east,south,west.
	
	// Used for multi-threading.
	bool started = false;	//Has a thread started working on this chunk?
	bool finished = false;   //Has a thread finished working on this chunk?
	bool visible = false;	//Should this chunk be rendered?


	Chunk();
	Chunk(Vector2 pos);
	~Chunk();


	void Draw();

	void Generate();

	void makeHill(Vector2 pos, float dy, float rad, UINT id);

	void stitch(UINT sKey);

	void shift(Direction dir);
};

