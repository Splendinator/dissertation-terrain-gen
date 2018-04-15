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

	//Initialise noise generators.
	generator = new Generator(-1000.0f, 1000.0f, 1800.0f);
	generator2 = new Generator(-1200.0f, 1200.0f, 300.0f);
	generator3 = new Generator(-4000.0f, 4000.0f, 750.0f);
	generator4 = new Generator(-200.0f, 200.0f, 60.0f);
	
	//Desert Generators
	generator5 = new Generator(-2700, 900, 1000);
	generator6 = new Generator(-750, 250, 160);

	//Biome map
	biomeMap = BiomeMap(2240, 820);

	//Initialising the threads.
	for (int i = 0; i < MAX_THREADS; i++) {
		t[i] = thread(&Renderer::threadLoop,this,i,(unsigned long long)chunk);
		t[i].detach();
	}


	//Initialise camera position middle of centre chunk.
	camera =			new Camera();
	camera->SetPosition(Vector3(CHUNK_SIZE * MAX_CHUNKS / 2, HEIGHTMAP_Y*RAW_HEIGHT, CHUNK_SIZE * MAX_CHUNKS / 2));		
	

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
	delete chunk;
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
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "time"),time);

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
	while (true) {
		for (int i = 0; i < MAX_CHUNKS; i++) {
			for (int j = 0; j < MAX_CHUNKS; j++) {
				if (!chunk[i * MAX_CHUNKS + j]->started && !flush)	//If a thread hasn't started work on chunk[i][j] yet...
				{
					chunk[i * MAX_CHUNKS + j]->started = true; //Signal that this thread has started
					

					for (int x = 0; x < RAW_HEIGHT; x++) {
						for (int y = 0; y < RAW_WIDTH; y++) {

							if (flush || !chunk[i * MAX_CHUNKS + j]->started) break;	//Hard stop





						//Worley
							Generator textureGenerator(0, 0.2, 87);
							Generator textureGenerator2(0, 0.1, 200);
							Generator textureGenerator3(0, 0.1, 54);
							BiomeMap worley = BiomeMap(200, 150);

							chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y =
								worley.getWorley(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1)
									, 200, 1100, 300, 1);
							chunk[i * MAX_CHUNKS + j]->h->shadePct[x + RAW_WIDTH*y] = 
								  textureGenerator.perlin(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1))
								+ textureGenerator2.perlin(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1))
								+ textureGenerator3.perlin(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1));

							chunk[i * MAX_CHUNKS + j]->h->water[x + RAW_WIDTH*y] = 1.f;
						//Snowy Hills
						/*		chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y =									//Add together multiple noises.
								  generator->simplex(y + (cameraPosX + i)*(RAW_WIDTH-1), x + (cameraPosY + j)*(RAW_HEIGHT-1))
								+ generator->perlin(y + (cameraPosX + i)*(RAW_WIDTH-1), x + (cameraPosY + j)*(RAW_HEIGHT-1))
								+ generator2->perlin(y + (cameraPosX + i)*(RAW_WIDTH-1), x + (cameraPosY + j)*(RAW_HEIGHT-1))
								+ generator3->perlin(y + (cameraPosX + i)*(RAW_WIDTH-1), x + (cameraPosY + j)*(RAW_HEIGHT-1));
						*/


						//Rocky hills;
						/*	float dx, dy;

							chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y = generator->simplexD(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1),&dx,&dy);
							chunk[i * MAX_CHUNKS + j]->h->gradients[x + RAW_WIDTH*y] = Vector2(dx, dy);
						*/


						//Biomes
							
							//float biomePct[MAX_BIOMES];
							//biomeMap.getBiome(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1) , biomePct);


							//float dx = 0, dy = 0;
							//float tx, ty;

					
							////chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y = 1200 * biomePct[0];
							//chunk[i * MAX_CHUNKS + j]->h->biomes[x + RAW_WIDTH*y].x = biomePct[0];
							//chunk[i * MAX_CHUNKS + j]->h->biomes[x + RAW_WIDTH*y].y = biomePct[1];
							//chunk[i * MAX_CHUNKS + j]->h->biomes[x + RAW_WIDTH*y].z = biomePct[2];

							//chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y = 0;

							////Field
							//if (biomePct[0] > 0) {
							//	chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y += (biomePct[0] * generator->perlin(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1)));

							//}
							//
							////Hills
							//if (biomePct[1] > 0) {

							//	chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y += (biomePct[1] * generator3->simplexD(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1), &tx, &ty));
							//	dx += biomePct[1] * tx;
							//	dy += biomePct[1] * ty;

							//	float grad = sqrt(dx * dx + dy * dy); //Gradient Magnitude
							//	float gradMult;

							//	if (grad > 3.0f) {
							//		gradMult = min((grad - 3.0f) / 3.0f, 1);
							//		//cout << grad << " " << gradMult << endl;
							//		chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y += (gradMult * (biomePct[1] * generator2->simplexD(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1), &tx, &ty)));
							//		dx += gradMult * biomePct[1] * tx;
							//		dy += gradMult * biomePct[1] * ty;
							//	}

							//	grad = sqrt(dx * dx + dy * dy); //Gradient Magnitude
							//	
							//	if (grad > 6.0f) {
							//		gradMult = min((grad - 6.0f) / 4.0f, 1);
							//		chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y += (gradMult * (biomePct[1] * generator4->simplexD(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1), &tx, &ty)));
							//		dx += gradMult * biomePct[1] * tx;
							//		dy += gradMult * biomePct[1] * ty;
							//	}

							//	chunk[i * MAX_CHUNKS + j]->h->gradients[x + RAW_WIDTH*y] = Vector2(dx, dy);
							//
							//}

							////Desert
							//if (biomePct[2] > 0) {
							//	chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y += (biomePct[2] * generator5->perlin(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1)));
							//	chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y += (biomePct[2] * generator6->perlin(y + (cameraPosX + i)*(RAW_WIDTH - 1), x + (cameraPosY + j)*(RAW_HEIGHT - 1)));

							//	//Domain Warping - Sand dunes
							//	chunk[i * MAX_CHUNKS + j]->h->vertices[x + RAW_WIDTH*y].y += 
							//		(biomePct[2] *
							//			generator6->perlin(
							//				generator6->perlin((y + (cameraPosX + i)*(RAW_WIDTH - 1) - 1), (x + (cameraPosY + j)*(RAW_HEIGHT - 1) + 6) * 0.15) * 1.1,
							//				generator6->perlin((y + (cameraPosX + i)*(RAW_WIDTH - 1) + 9), (x + (cameraPosY + j)*(RAW_HEIGHT - 1) - 2) * 0.15) * 1.1
							//			) * 5
							//		);
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
						if (flush || !chunk[i * MAX_CHUNKS + j]->started) break;	//Hard stop
					}

					if (flush || !chunk[i * MAX_CHUNKS + j]->started)	//Re-flag chunk for generation because it was halted half way through and is likely corrupt (Reliant on cameraPosY/cameraPosX not changing half way through).
					{
						chunk[i * MAX_CHUNKS + j]->started = false;
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