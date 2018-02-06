#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/camera.h"
#include "../../nclgl/HeightMap.h"
#include "Chunk.h"
#include "Generator.h"
#include <thread>


class Renderer : public OGLRenderer {
private:
	static const int MAX_CHUNKS = 19;	//The maximum ammount of chunks that will be rendered every frame. (3 means 3x3 grid around player)
	static const int MAX_THREADS = 7;	//The number of threads that we can use. More = faster terrain generation but more CPU usage.
	int cameraPosX = MAX_CHUNKS / 2, cameraPosY = MAX_CHUNKS / 2;
	
	
	Chunk *chunk[MAX_CHUNKS*MAX_CHUNKS]; //array of pointers to all the chunks;


	thread t[MAX_THREADS]; //Threads used to calculate noise without freezing game.
	bool flush = false; //Used to force-stop all thread terrain generation. Needs to be toggled true if you ever edit chunk pointers.


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
