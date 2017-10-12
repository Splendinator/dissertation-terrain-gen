#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/camera.h"
#include "../../nclgl/HeightMap.h"
#include "Chunk.h"

class Renderer : public OGLRenderer {
private:
	static const int MAX_CHUNKS = 9;	//The maximum ammount of chunks that will be rendered every frame. (9 means 3x3 grid around player)

public :
	Renderer ( Window & parent );
	 virtual ~ Renderer ( void );
	
	 virtual void RenderScene ();
	 virtual void UpdateScene ( float msec );
	
	 Chunk * chunk[MAX_CHUNKS];
	 Chunk * getActiveChunk();			//Get chunk that camera is above

	 Camera * camera ;
 };
