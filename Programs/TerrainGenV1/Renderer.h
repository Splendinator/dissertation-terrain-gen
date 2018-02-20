#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/camera.h"
#include "../../nclgl/HeightMap.h"
#include "Chunk.h"
#include "Generator.h"
#include "BiomeMap.h"
#include <thread>


class Renderer : public OGLRenderer {
private:
	static const int MAX_CHUNKS = 7;	//The maximum ammount of chunks that will be rendered every frame. (3 means 3x3 grid around player)
	static const int MAX_THREADS = 4;	//The number of threads that we can use. More = faster terrain generation but more CPU usage.
	int cameraPosX = MAX_CHUNKS / 2, cameraPosY = MAX_CHUNKS / 2; // Chunks that the camera is above.
	
	
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
	 Generator * generator4;
	 Generator * generator5;
	 Generator * generator6;
	 Generator * generator7;
	 Generator * generator8;
	 BiomeMap biomeMap;

	 void shiftChunks(Direction dir);
	 void setPointers();
	 void threadLoop(int id, unsigned long long);
};
