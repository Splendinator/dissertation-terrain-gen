#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/camera.h"
#include "../../nclgl/HeightMap.h"
#include "Chunk.h"
#include "Generator.h"


class Renderer : public OGLRenderer {
private:
	static const int MAX_CHUNKS = 3;	//The maximum ammount of chunks that will be rendered every frame. (3 means 3x3 grid around player)
	int cameraPosX = MAX_CHUNKS / 2, cameraPosY = MAX_CHUNKS / 2;

public :
	Renderer ( Window & parent );
	 virtual ~ Renderer ( void );
	
	 virtual void RenderScene ();
	 virtual void UpdateScene ( float msec );
	
	 Chunk * chunk[MAX_CHUNKS][MAX_CHUNKS];
	 Chunk * getActiveChunk();			//Get chunk that camera is above

	 Camera * camera ;
	 Generator * generator;

	 void shiftChunks(Direction dir);
	 void setPointers();
	 void perlinGen(const int &x, const int &y);
};
