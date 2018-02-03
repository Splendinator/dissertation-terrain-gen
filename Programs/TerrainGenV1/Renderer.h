#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/camera.h"
#include "../../nclgl/HeightMap.h"
#include "Chunk.h"
#include "Generator.h"
#include <thread>


class Renderer : public OGLRenderer {
private:
	static const int MAX_CHUNKS = 7;	//The maximum ammount of chunks that will be rendered every frame. (3 means 3x3 grid around player)
	static const int MAX_THREADS = 4;	//The number of threads that we can use. More = faster terrain generation but laggier player movement during terrain generation.
	int cameraPosX = MAX_CHUNKS / 2, cameraPosY = MAX_CHUNKS / 2;
	
	Chunk *chunk = new Chunk[MAX_CHUNKS*MAX_CHUNKS]; //array of all the chunks;


	thread t[MAX_THREADS]; //Threads used to calculate noise without freezing game.

public :
	Renderer ( Window & parent );
	 virtual ~ Renderer ( void );
	
	 virtual void RenderScene ();
	 virtual void UpdateScene ( float msec );
	
	 
	 Chunk * getActiveChunk();			//Get chunk that camera is above

	 Camera * camera ;
	 Generator * generator;
	 Generator * generator2;
	 Generator * generator3;

	 void shiftChunks(Direction dir);
	 void setPointers();
	 void threadLoop(int id, unsigned long long);
};
