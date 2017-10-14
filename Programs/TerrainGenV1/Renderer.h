#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/camera.h"
#include "../../nclgl/HeightMap.h"
#include "Chunk.h"


class Renderer : public OGLRenderer {
private:
	static const int MAX_CHUNKS = 5;	//The maximum ammount of chunks that will be rendered every frame. (3 means 3x3 grid around player)

public :
	Renderer ( Window & parent );
	 virtual ~ Renderer ( void );
	
	 virtual void RenderScene ();
	 virtual void UpdateScene ( float msec );
	
	 Chunk * chunk[MAX_CHUNKS][MAX_CHUNKS];
	 Chunk * getActiveChunk();			//Get chunk that camera is above

	 Camera * camera ;
	 void shiftChunks(Direction dir);
	 void setPointers();
 };
