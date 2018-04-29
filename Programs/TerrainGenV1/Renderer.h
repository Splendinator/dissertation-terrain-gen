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
	 Generator generator = Generator(-300.0f, 300.0f, 120.0f); //islands in ISLANDS
	 Generator generator2 = Generator(0,6000,600.f);	//mountains in HILLS
	 Generator generator3 = Generator(-120, 180, 52.f); //mountains in HILLS
	 Generator generator4 = Generator(-50, 100, 23.f);  //mountains in HILLS
	 Generator generator5 = Generator(0, 1, 2000.f);	//used for perlinFilter
	 Generator generator6 = Generator(-200, 400, 700);	//noise for DESERT
	 Generator generator7 = Generator(-150,0,250);		//noise for DESERT
	 Generator generator8 = Generator(0, 1000, 400);	//noise for domain warping in DESERT
	 Generator generator9 = Generator(0, 900, 280);	//noise for FIELD

	 Generator generatorVoronoi = Generator(-120, 120, 230);

	 Generator generatorTexCoords = Generator(-0.5, 0.5, 300);

	 Generator textureGenerator = Generator(0, 0.21, 800);
	 Generator textureGenerator2 = Generator(0, 0.18, 240);
	 Generator textureGenerator3 = Generator(0,0.15,14);

	 BiomeMap worley = BiomeMap(200, 150);
	 BiomeMap biomeMap = BiomeMap(3240,1320);
	 BiomeMap biomeMap2 = BiomeMap(324, 132);

	 Generator temp = Generator(200, 1200, 300);
	 Generator heightTex = Generator(-300, 300, 200);
	 Generator gradTex = Generator(-13, 13, 80);

	 void shiftChunks(Direction dir);
	 void setPointers();
	 void threadLoop(int id, unsigned long long);
	 void generateTerrain(int cX, int cY, int vX, int vY);	//Chunk X, Chunk Y, Verticie X, Verticie Y;
	 void lerpVert(Chunk *c, int v1, int v2, int v3, float pct);	//Lineraly Interpolate texture, water, and height from v1 and v2 into v3
};
