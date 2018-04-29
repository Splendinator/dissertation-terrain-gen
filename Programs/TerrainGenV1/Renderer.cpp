#include <iostream>
#include <random>
#include <thread>
#include "Renderer.h"



/*BUGS:		Runtime error with too many (>23 on my machine) chunks

			Going exactly diagonally causes concurrency errors.

*/

Renderer::Renderer(Window & parent) : OGLRenderer(parent) {

	//Initialise the chunks.
	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			chunk[i*MAX_CHUNKS+j] = new Chunk(Vector2(i,j));
		}
	}

	

	setPointers();

	

	//Initialising the threads.
	for (int i = 0; i < MAX_THREADS; i++) {
		t[i] = thread(&Renderer::threadLoop,this,i,(unsigned long long)chunk);
		t[i].detach();
	}


	//Initialise camera position middle of centre chunk.
	camera =			new Camera();
	camera->SetPosition(Vector3((CHUNK_SIZE * MAX_CHUNKS / 2), HEIGHTMAP_Y*RAW_HEIGHT, (CHUNK_SIZE * MAX_CHUNKS / 2)));	

	currentShader =		new Shader("../../Shaders/TexturedVertex.glsl",
						"../../Shaders/TexturedFragment.glsl");

	if (!currentShader->LinkProgram()) {
		return;
	}

	currentShader->GetProgram();

	projMatrix = Matrix4::Perspective(1.0f, 100000.0f,
		(float)width / (float)height, 55.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);


	init = true;

}

Renderer ::~Renderer(void) {
	delete[] chunk;
	delete camera;
}



void Renderer::UpdateScene(float msec) {
	static float time = 0;
	time += msec;

	camera->UpdateCamera(msec);
	
	if (camera->GetPosition().x < 0 + CHUNK_SIZE * (int)(MAX_CHUNKS / 2))
	{ 
		camera->SetPosition(camera->GetPosition() + Vector3(CHUNK_SIZE, 0.0f, 0.0f));
		cameraPosX--;
		shiftChunks(WEST);
	}
	else if (camera->GetPosition().x > CHUNK_SIZE + CHUNK_SIZE * (int)(MAX_CHUNKS/2))
	{ 
		camera->SetPosition(camera->GetPosition() + Vector3(-CHUNK_SIZE, 0.0f, 0.0f));
		cameraPosX++;
		shiftChunks(EAST);
	}
	if		(camera->GetPosition().z < 0 + CHUNK_SIZE * (int)(MAX_CHUNKS / 2))
	{ 
		camera->SetPosition(camera->GetPosition() + Vector3(0.0f, 0.0f, CHUNK_SIZE));
		cameraPosY--;
		shiftChunks(NORTH);	
	}
	else if (camera->GetPosition().z > CHUNK_SIZE + CHUNK_SIZE * (int)(MAX_CHUNKS / 2)) 
	{ 
		camera->SetPosition(camera->GetPosition() + Vector3(0.0f, 0.0f, -CHUNK_SIZE));
		cameraPosY++;
		shiftChunks(SOUTH);	
	}

	glUseProgram(currentShader->GetProgram());
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "time"), time);	//Send time to GPU
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "activeChunkCoords"), cameraPosX * RAW_WIDTH * HEIGHTMAP_X,cameraPosY * RAW_HEIGHT * HEIGHTMAP_Z); //Send world co-ordinates to GPU for use with water waves.

	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(currentShader->GetProgram());
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"grassTex"), 0);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"snowTex"), 1);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"rockTex"), 2);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"sandTex"), 3);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"waterTex"), 4);

	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			
			if (chunk[i * MAX_CHUNKS + j]->finished && !chunk[i * MAX_CHUNKS + j]->visible) {	//First frame a chunk becomes visible...
				chunk[i * MAX_CHUNKS + j]->h->BufferData();	//Buffer the chunk data (has to be done on main thread)
				chunk[i * MAX_CHUNKS + j]->Draw();
				chunk[i * MAX_CHUNKS + j]->visible = true;	//Set it visible so we don't re-buffer the same data next frame.
			}
			else if (chunk[i * MAX_CHUNKS + j]->visible) {	//If chunk is visible just draw it like normal.
				chunk[i * MAX_CHUNKS + j]->Draw();
			}
		}
	}
	

	//TODO: Water maybe?
	//glUseProgram(waterShader->GetProgram());	
	//water->Draw();


	glUseProgram(0);
	SwapBuffers();
}

Chunk * Renderer::getActiveChunk() {
	return chunk[MAX_CHUNKS/2 * MAX_CHUNKS + MAX_CHUNKS/2];
}


void Renderer::shiftChunks(Direction dir) {
	int offset;
	Chunk *temp[MAX_CHUNKS]; //Temp storage of pointers because we have cyclic dependencies.
	flush = true;//Temperarily stop terrain generation while we mess around with pointers.

	switch(dir){
		case(WEST): {
			
			//Edge case 1 - Westmost pointers have to wrap round and point to eastmost chunks. They also need to store their values in a temp array for later use.
			for (int j = 0; j < MAX_CHUNKS; j++) {
				temp[j] = chunk[0 * MAX_CHUNKS + j];	
				chunk[0 * MAX_CHUNKS + j] = chunk[(MAX_CHUNKS - 1) * MAX_CHUNKS + j];
				chunk[0 * MAX_CHUNKS + j]->finished = chunk[0 * MAX_CHUNKS + j]->started = chunk[0 * MAX_CHUNKS + j]->visible = false;
			}

			//Normal case - Chunks pointers point one to the east (opposite to player to give illusion of movement)
			for (int i = MAX_CHUNKS-1; i >= 1 ; i--) {
				for (int j = 0; j < MAX_CHUNKS; j++) {
					chunk[i * MAX_CHUNKS + j] =  chunk[(i - 1) * MAX_CHUNKS + j];
				}
			}
			
			//Edge case 2 - Final row has to point to the temporary array.
			for (int j = 0; j < MAX_CHUNKS; j++) {
				chunk[1 * MAX_CHUNKS + j] = temp[j];
			}

			break;
		}
		 
		case(EAST): {

			//Edge case 1
			for (int j = 0; j < MAX_CHUNKS; j++) {
				temp[j] = chunk[(MAX_CHUNKS - 1) * MAX_CHUNKS + j];
				chunk[(MAX_CHUNKS - 1) * MAX_CHUNKS + j] = chunk[0 * MAX_CHUNKS + j];
				chunk[(MAX_CHUNKS - 1) * MAX_CHUNKS + j]->finished = chunk[(MAX_CHUNKS - 1) * MAX_CHUNKS + j]->started = chunk[(MAX_CHUNKS - 1) * MAX_CHUNKS + j]->visible = false;
			}

			//Normal case
			for (int i = 0; i <= (MAX_CHUNKS - 2); i++) {
				for (int j = 0; j < MAX_CHUNKS; j++) {
					chunk[i * MAX_CHUNKS + j] = chunk[(i + 1) * MAX_CHUNKS + j];
				}
			}

			//Edge case 2
			for (int j = 0; j < MAX_CHUNKS; j++) {
				chunk[(MAX_CHUNKS - 2) * MAX_CHUNKS + j] = temp[j];
			}
			break;
		}

		case(NORTH): {

			//Edge case 1
			for (int i = 0; i < MAX_CHUNKS; i++) {
				temp[i] = chunk[i * MAX_CHUNKS + 0];
				chunk[i * MAX_CHUNKS + 0] = chunk[i * MAX_CHUNKS + (MAX_CHUNKS-1)];
				chunk[i * MAX_CHUNKS + 0]->finished = chunk[i * MAX_CHUNKS + 0]->started = chunk[i * MAX_CHUNKS + 0]->visible = false;
			}

			//Normal case
			for (int i = 0; i < MAX_CHUNKS; i++) {
				for (int j = MAX_CHUNKS-1; j >= 1; j--) {
					chunk[i * MAX_CHUNKS + j] = chunk[i * MAX_CHUNKS + (j-1)];
				}
			}

			//Edge case 2
			for (int i = 0; i < MAX_CHUNKS; i++) {
				chunk[i * MAX_CHUNKS + 1] = temp[i];
			}
			break;
		}

		case(SOUTH): {

			//Edge case 1
			for (int i = 0; i < MAX_CHUNKS; i++) {
				temp[i] = chunk[i * MAX_CHUNKS + (MAX_CHUNKS-1)];
				chunk[i * MAX_CHUNKS + (MAX_CHUNKS - 1)] = chunk[i * MAX_CHUNKS + 0];
				chunk[i * MAX_CHUNKS + (MAX_CHUNKS - 1)]->finished = chunk[i * MAX_CHUNKS + (MAX_CHUNKS - 1)]->started = chunk[i * MAX_CHUNKS + (MAX_CHUNKS - 1)]->visible = false;
			}

			//Normal case
			for (int i = 0; i < MAX_CHUNKS; i++) {
				for (int j = 0; j <= MAX_CHUNKS-2; j++) {
					chunk[i * MAX_CHUNKS + j] = chunk[i * MAX_CHUNKS + (j + 1)];
				}
			}

			//Edge case 2
			for (int i = 0; i < MAX_CHUNKS; i++) {
				chunk[i * MAX_CHUNKS + (MAX_CHUNKS-2)] = temp[i];
			}
			break;
		}

	}

	
	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			chunk[i * MAX_CHUNKS + j]->wc = Vector2(i, j); //Re-calculate the world coordinates of each chunk because an unwanted side effect of changing pointers is changing the position of a chunk.
			if (chunk[i * MAX_CHUNKS + j]->started && !chunk[i * MAX_CHUNKS + j]->finished) chunk[i * MAX_CHUNKS + j]->started = false; //If any chunk was mid way through generation before we changed pointers, just re-start it.
		}
	}

	flush = false;	//Let terrain generation continue.

}

//Set the pointer for all the chunks
void Renderer::setPointers() {
	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			if (i > 0)					chunk[i * MAX_CHUNKS + j]->w = chunk[(i - 1) * MAX_CHUNKS + j];
			if (i < MAX_CHUNKS - 1)		chunk[i * MAX_CHUNKS + j]->e = chunk[(i + 1) * MAX_CHUNKS + j];
			if (j > 0)					chunk[i * MAX_CHUNKS + j]->n = chunk[i * MAX_CHUNKS + (j - 1)];
			if (j < MAX_CHUNKS - 1)		chunk[i * MAX_CHUNKS + j]->s = chunk[i * MAX_CHUNKS + (j + 1)];
		}
	}
}



//TODO: Render in spiral pattern around player (so that closer chunks render first)
void Renderer::threadLoop(int id, unsigned long long c) {
	restartLoop:
	while (true) {
		for (int i = 0; i < MAX_CHUNKS; i++) {
			for (int j = 0; j < MAX_CHUNKS; j++) {
				if (!chunk[i * MAX_CHUNKS + j]->started && !flush)	//If a thread hasn't started work on chunk[i][j] yet...
				{
					chunk[i * MAX_CHUNKS + j]->started = id+1; //Signal that this thread has started work
					int LoD = 1;	//Level of detail; *** VALID VALUES ARE {1,2,4,8,16,32,64,128} ***

					//Edges; Must do all to ensure heightmaps seamlessly stitch together
					//for (int x = 0; x < RAW_HEIGHT; x++) {
					//	
					//	if (flush || !chunk[i * MAX_CHUNKS + j]->started) break;	//Hard stop
					//	generateTerrain(i, j, 0, x);
					//	generateTerrain(i, j, x, 0);
					//	generateTerrain(i, j, RAW_HEIGHT-1, x);
					//	generateTerrain(i, j, x, RAW_HEIGHT-1);
					//
					//}

					//LoD Vertices                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            v
					for (int x = 0; x < RAW_HEIGHT; x++) {
						for (int y = 0; y < RAW_WIDTH; y++) {
							if (flush || chunk[i * MAX_CHUNKS + j]->started != id+1) goto exitSafely; //Hard stop
							
							generateTerrain(i, j, x, y);
							chunk[i * MAX_CHUNKS + j]->h->texturePct[x + RAW_WIDTH*y].x = 1 - float(id)/4;
							chunk[i * MAX_CHUNKS + j]->h->texturePct[x + RAW_WIDTH*y].y = 0;
							chunk[i * MAX_CHUNKS + j]->h->texturePct[x + RAW_WIDTH*y].z = float(id)/4;
							chunk[i * MAX_CHUNKS + j]->h->texturePct[x + RAW_WIDTH*y].w = 0;
						}
						
					}

					

					/// LEVEL OF DETAIL
					////Adjascent vertices linearly interpolated
					//for (int x = 0; x < (RAW_WIDTH-1); x += LoD) {
					//	for (int y = LoD; y < (RAW_WIDTH - 1); y += LoD) {
					//		for (int z = 1; z < LoD; z++) {
					//			if (flush || !chunk[i * MAX_CHUNKS + j]->started) break;	//Hard stop
					//			lerpVert(chunk[i * MAX_CHUNKS + j], x + RAW_WIDTH*y, x + LoD + RAW_WIDTH*y, x + RAW_WIDTH*y + z,float(z)/LoD);
					//			lerpVert(chunk[i * MAX_CHUNKS + j], y + RAW_WIDTH*x, y + RAW_WIDTH*(x + LoD), y + RAW_WIDTH*(x + z), float(z) / LoD);
					//			//chunk[i * MAX_CHUNKS + j]->h->vertices[y + RAW_WIDTH*(x + z)].y =
					//			//	chunk[i * MAX_CHUNKS + j]->h->vertices[y + RAW_WIDTH*x].y +
					//			//	(float(z) / LoD) * (chunk[i * MAX_CHUNKS + j]->h->vertices[y + RAW_WIDTH*(x + LoD)].y - chunk[i * MAX_CHUNKS + j]->h->vertices[y + RAW_WIDTH*x].y);

					//		}
					//	}
					//}

					////Vertices linearly interpolated
					//for (int x = 0; x < (RAW_WIDTH - 1); x += LoD) {
					//	for (int y = 0; y < (RAW_WIDTH - 1); y += LoD) {
					//		for (int z = 1; z < LoD; z++) {
					//			for (int w = 1; w < LoD; w++) {
					//				if (flush || !chunk[i * MAX_CHUNKS + j]->started) break;	//Hard stop
					//				lerpVert(chunk[i * MAX_CHUNKS + j], x + RAW_WIDTH*(y + w), x + LoD + RAW_WIDTH*(y + w), (x + z) + RAW_WIDTH*(y + w), float(z) / LoD);
					//				/*chunk[i * MAX_CHUNKS + j]->h->vertices[(x+z) + RAW_WIDTH*(y+w)].y =
					//					chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*(y + w)].y +
					//					(float(z) / LoD) * (chunk[i * MAX_CHUNKS + j]->h->vertices[x + LoD + RAW_WIDTH*(y + w)].y - chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*(y + w)].y);*/
					//			}
					//		}
					//	}
					//}

					if (flush || chunk[i * MAX_CHUNKS + j]->started != id + 1)	//Re-flag chunk for generation because it was halted half way through and is likely corrupt (Reliant on cameraPosY/cameraPosX not changing half way through).
					{
					exitSafely:
						if (chunk[i * MAX_CHUNKS + j]->started > 0) goto restartLoop;	//If this line is reached another thread started generating this chunk at the same time, so we just heed and let it finish.
						chunk[i * MAX_CHUNKS + j]->started = 0;
						chunk[i * MAX_CHUNKS + j]->finished = false;


					}
					else
					{
						chunk[i * MAX_CHUNKS + j]->finished = true;	//Let main program know this chunk has generated
					}
				}
			}
		}
	}
}


void Renderer::generateTerrain(int cX, int cY, int vX, int vY) {
															
	chunk[cX * MAX_CHUNKS + cY]->h->water[vX + RAW_WIDTH*vY] = 0; //Get rid of water

	float biomePct[MAX_BIOMES];
	biomeMap2.getBiome(vY + (cameraPosX + cX)*(RAW_WIDTH - 1) + generatorVoronoi.perlin(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1)),
					   vX + (cameraPosY + cY)*(RAW_HEIGHT - 1) + generatorVoronoi.perlin(vX + (cameraPosY + cY)*(RAW_HEIGHT - 1), vY + (cameraPosX + cX)*(RAW_WIDTH - 1)),biomePct);

	biomePct[1] = biomePct[2] = biomePct[0] = 0;
	biomePct[3] = 1;

	float dx = 0, dy = 0;
	float tx, ty;

	
	//chunk[cX * MAX_CHUNKS + cY]->h->textureCoords[vX + RAW_WIDTH*vY].x += generatorTexCoords.perlin(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1));
	//chunk[cX * MAX_CHUNKS + cY]->h->textureCoords[vX + RAW_WIDTH*vY].y += generatorTexCoords.perlin(vX + (cameraPosY + cY)*(RAW_HEIGHT - 1), vY + (cameraPosX + cX)*(RAW_WIDTH - 1));


	float perlinFilter = generator5.perlin(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1));

	//chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y = biomePct[1] * 500 + biomePct[2] * 1000 + biomePct[3] * 1500;

	chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y = 0;
	//chunk[cX * MAX_CHUNKS + cY]->h->water[vX + RAW_WIDTH*vY] = 1.0f;

	//chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y = temp.simplexD(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1),&dx,&dy);

	//Shading
	//chunk[cX * MAX_CHUNKS + cY]->h->shadePct[vX + RAW_WIDTH*vY] =
	//	textureGenerator.perlin(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1))
	//	+ textureGenerator2.perlin(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1))
	//	+ textureGenerator3.perlin(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1));
	

	chunk[cX * MAX_CHUNKS + cY]->h->texturePct[vX + RAW_WIDTH*vY] = Vector4(biomePct[0], biomePct[1], biomePct[2], biomePct[3]);

	//
	//
	////***ISLANDS***
	//if (biomePct[ISLAND] > 0) {
	//	chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y += ((1050 -
	//		worley.getWorley(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1)
	//			, 200, 1100, 400, 3)
	//		+ generator.perlin(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1))
	//		) - (1200 * perlinFilter)) * biomePct[ISLAND];
	//
	//
	//
	//	//underwater - Water gets rougher and darker the deeper it gets.
	//	if (chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y < 0) {
	//		if (biomePct[ISLAND] == 1) {
	//			chunk[cX * MAX_CHUNKS + cY]->h->water[vX + RAW_WIDTH*vY] = min(1.0f, abs(chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y / 600));
	//			chunk[cX * MAX_CHUNKS + cY]->h->shadePct[vX + RAW_WIDTH*vY] += min(0.2f, abs(chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y / (600 * 5)));// + 0.3f;
	//		}
	//		//chunk[cX * MAX_CHUNKS + cY]->h->shadePct[vX + RAW_WIDTH*vY] += 0.15f;
	//		chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y = 0;
	//	}
	//
	//	//sand near the water. (Should really be mud in a swamp)
	//	float sandVal = max(min((80.0f - chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y) / 30, 1), 0);
	//	chunk[cX * MAX_CHUNKS + cY]->h->texturePct[vX + RAW_WIDTH*vY] = Vector4(1 - sandVal * biomePct[ISLAND], sandVal * biomePct[ISLAND], 0, 0);
	//	chunk[cX * MAX_CHUNKS + cY]->h->shadePct[vX + RAW_WIDTH*vY] += 0.12f * (1 - sandVal);
	//}
	//
	//
	////***HILLS***
	//if (biomePct[HILLS] > 0) {
	//	chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y += (biomePct[HILLS] * generator2.simplexD(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1), &tx, &ty) * (perlinFilter * 3 - 1.2));
	//	dx += biomePct[HILLS] * tx * (perlinFilter * 3 - 1.2);
	//	dy += biomePct[HILLS] * ty * (perlinFilter * 3 - 1.2);
	//
	//	float grad = sqrt(dx * dx + dy * dy);
	//	float gradMult;
	//
	//	//Snow texture
	//	float snowVal = max(min((-25.f + chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y) / 300.f, 1), 0);
	//	chunk[cX * MAX_CHUNKS + cY]->h->texturePct[vX + RAW_WIDTH*vY] = Vector4((1 - snowVal) * biomePct[HILLS], 0, (snowVal)* biomePct[HILLS], 0);
	//
	//	if (grad > 2) {
	//		gradMult = min((grad - 2) / 10.0f, 1);
	//		chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y += (biomePct[HILLS] * generator3.simplexD(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1), &tx, &ty)) * gradMult;
	//		dx += biomePct[HILLS] * tx * gradMult;
	//		dy += biomePct[HILLS] * ty * gradMult;
	//		grad = sqrt(dx * dx + dy * dy);
	//	}
	//
	//	if (grad > 3) {
	//		gradMult = min((grad - 3) / 15.0f, 1);
	//		chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y += (biomePct[HILLS] * generator4.simplexD(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1), &tx, &ty)) * gradMult;
	//		dx += biomePct[HILLS] * tx * gradMult;
	//		dy += biomePct[HILLS] * ty * gradMult;
	//		grad = sqrt(dx * dx + dy * dy);
	//	}
	//
	//	gradMult = min((grad - 9) / 5.0f, 1);
	//	chunk[cX * MAX_CHUNKS + cY]->h->texturePct[vX + RAW_WIDTH*vY].x *= 1 - gradMult;
	//	chunk[cX * MAX_CHUNKS + cY]->h->texturePct[vX + RAW_WIDTH*vY].z *= 1 - gradMult;
	//	chunk[cX * MAX_CHUNKS + cY]->h->texturePct[vX + RAW_WIDTH*vY].w = gradMult;
	//
	//}
	//
	////***DESERT***
	//if (biomePct[DESERT] > 0) {
	//
	//	chunk[cX * MAX_CHUNKS + cY]->h->texturePct[vX + RAW_WIDTH*vY] = Vector4(0, biomePct[DESERT], 0, 0);
	//	chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y += (biomePct[DESERT] * generator6.perlin(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1)));
	//	chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y += (biomePct[DESERT] * generator7.perlin(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1)));
	//
	//	//Domain Warping - Sand dunes
	//	chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y +=
	//		(biomePct[DESERT] *
	//			generator8.perlin(
	//				generator8.perlin((vY + (cameraPosX + cX)*(RAW_WIDTH - 1) - 16), (vX + (cameraPosY + cY)*(RAW_HEIGHT - 1) + 9) * 0.3f) * 1.1f,
	//				generator8.perlin((vY + (cameraPosX + cX)*(RAW_WIDTH - 1) + 13), (vX + (cameraPosY + cY)*(RAW_HEIGHT - 1) - 6) * 0.3f) * 1.1f
	//			) * 5.f
	//			);
	//}
	//
	////*** FIELD ***
	//if (biomePct[FIELD] > 0) {
	//	chunk[cX * MAX_CHUNKS + cY]->h->texturePct[vX + RAW_WIDTH*vY] = Vector4(biomePct[FIELD], 0, 0, 0);
	//	chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y += (biomePct[FIELD] * generator9.simplex(vY + (cameraPosX + cX)*(RAW_WIDTH - 1), vX + (cameraPosY + cY)*(RAW_HEIGHT - 1)));
	//	//if (chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y > 300) {
	//	//	float cliff = min(((300 - chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y) / 100), 1);
	//	//	chunk[cX * MAX_CHUNKS + cY]->h->vertices[vX + RAW_WIDTH*vY].y += cliff * 2000;
	//	//}
	//}

	

	//3-layer domain warping	
	//generator6->perlin(
	//		generator6->perlin(
	//			generator6->perlin(y + (cameraPosX + i)*(RAW_WIDTH - 1) + 7, x + (cameraPosY + j)*(RAW_HEIGHT - 1) + 2) * 3,
	//			generator6->perlin(y + (cameraPosX + i)*(RAW_WIDTH - 1) - 5, x + (cameraPosY + j)*(RAW_HEIGHT - 1) + 1) * 3 
	//		)																										    * 3,
	//		generator6->perlin(
	//			generator6->perlin(y + (cameraPosX + i)*(RAW_WIDTH - 1) - 5, x + (cameraPosY + j)*(RAW_HEIGHT - 1) + 3) * 3,
	//			generator6->perlin(y + (cameraPosX + i)*(RAW_WIDTH - 1) + 2, x + (cameraPosY + j)*(RAW_HEIGHT - 1) + 0) * 3
	//		)																											* 3
	//	)																												* 5
	//	;

	//2-Layer domain warping
	//	generator6->perlin(
	//		generator6->perlin((y + (cameraPosX + i)*(RAW_WIDTH - 1) - 1), (x + (cameraPosY + j)*(RAW_HEIGHT - 1) + 6) * 0.15) * 1.1 , 
	//		generator6->perlin((y + (cameraPosX + i)*(RAW_WIDTH - 1) + 9), (x + (cameraPosY + j)*(RAW_HEIGHT - 1) - 2) * 0.15) * 1.1
	//	) * 5 + 400
	//	;
}

void Renderer::lerpVert(Chunk *c, int v1, int v2, int v3, float pct)
{
	c->h->vertices[v3].y =
		c->h->vertices[v1].y +
		pct * (c->h->vertices[v2].y - c->h->vertices[v1].y);

}
