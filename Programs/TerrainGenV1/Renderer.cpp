#include <iostream>
#include <random>
#include <thread>
#include "Renderer.h"






Renderer::Renderer(Window & parent) : OGLRenderer(parent) {

	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			chunk[i*MAX_CHUNKS+j] = Chunk(Vector2(i*(CHUNK_SIZE),j*(CHUNK_SIZE)));
			chunk[i*MAX_CHUNKS+j].started = chunk[i*MAX_CHUNKS + j].finished = chunk[i*MAX_CHUNKS + j].visible = false;
		}
	}

	

	//setPointers();

	generator = new Generator(-500.0f, 500.0f, 300.0f);
	generator2 = new Generator(-600.0f, 600.0f, 230.0f);
	generator3 = new Generator(-400.0f, 400.0f, 170.0f);

	for (int i = 0; i < MAX_THREADS; i++) {
		t[i] = thread(&Renderer::threadLoop,this,i,(unsigned long long)chunk);
		t[i].detach();
	}



	camera =			new Camera();
	camera->SetPosition(Vector3(CHUNK_SIZE * MAX_CHUNKS / 2, HEIGHTMAP_Y*RAW_HEIGHT, CHUNK_SIZE * MAX_CHUNKS / 2));
	
	currentShader =		new Shader("../../Shaders/TexturedVertex.glsl",
						"../../Shaders/TexturedFragment.glsl");

	if (!currentShader->LinkProgram()) {
		return;
	}

	currentShader->GetProgram();

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
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

	//if		(camera->GetPosition().x < 0 + CHUNK_SIZE * (int)(MAX_CHUNKS / 2))			{ camera->SetPosition(camera->GetPosition() + Vector3(CHUNK_SIZE, 0.0f, 0.0f));		cameraPosX--;	shiftChunks(WEST); 		}
	//else if (camera->GetPosition().x > CHUNK_SIZE + CHUNK_SIZE * (int)(MAX_CHUNKS/2))	{ camera->SetPosition(camera->GetPosition() + Vector3(-CHUNK_SIZE, 0.0f, 0.0f));	cameraPosX++;	shiftChunks(EAST); 		}
	//if		(camera->GetPosition().z < 0 + CHUNK_SIZE * (int)(MAX_CHUNKS / 2))			{ camera->SetPosition(camera->GetPosition() + Vector3(0.0f, 0.0f, CHUNK_SIZE));		cameraPosY--;	shiftChunks(NORTH);		}
	//else if (camera->GetPosition().z > CHUNK_SIZE + CHUNK_SIZE * (int)(MAX_CHUNKS / 2)) { camera->SetPosition(camera->GetPosition() + Vector3(0.0f, 0.0f, -CHUNK_SIZE));	cameraPosY++;	shiftChunks(SOUTH);		}
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

	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			
			if (chunk[i * MAX_CHUNKS + j].finished && !chunk[i * MAX_CHUNKS + j].visible) {
				chunk[i * MAX_CHUNKS + j].h->BufferData();	//Data buffering has to be done on the main thread, so we need this little check "if finished but not visible" to track the first frame a chunk is rendered.
				chunk[i * MAX_CHUNKS + j].Draw();						
				chunk[i * MAX_CHUNKS + j].visible = true;			//Set it visible so we don't re-buffer the same data next frame.
			}
			else if (chunk[i * MAX_CHUNKS + j].visible) {	//If chunk is visible just draw it like normal.
				chunk[i * MAX_CHUNKS + j].Draw();	
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
	return nullptr;
}

//TODO: Optimise this?
//TODO: OPTIMISATION IDEA - LET THE GPU HANDLE X and Z coordinates based off UNIFORM VEC2 WorldCoordinates. This would let you just change a pointer to the chunk instead of updating all the Y values. 
void Renderer::shiftChunks(Direction dir) {
	int offset;
	switch(dir){
		case(WEST): {
			for (int i = MAX_CHUNKS-1; i >= 1 ; i--) {
				for (int j = 0; j < MAX_CHUNKS; j++) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i * MAX_CHUNKS + j].h->vertices[k].y =  chunk[(i - 1) * MAX_CHUNKS + j].h->vertices[k].y;
					}

				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				chunk[i].started = chunk[i].finished = chunk[i].visible = false;
			}
			for (int i = 1; i < MAX_CHUNKS - 1; i++) {
			}
			break;
		}
		 
		case(EAST): {
			for (int i = 0; i < MAX_CHUNKS - 1; i++) {
				for (int j = 0; j < MAX_CHUNKS; j++) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i * MAX_CHUNKS + j].h->vertices[k].y = chunk[(i + 1) * MAX_CHUNKS + j].h->vertices[k].y;
					}
				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				//perlinGen(MAX_CHUNKS - 1, i);
				chunk[MAX_CHUNKS*(MAX_CHUNKS-1) + i].started = chunk[MAX_CHUNKS*(MAX_CHUNKS - 1) + i].finished = chunk[MAX_CHUNKS*(MAX_CHUNKS - 1) + i].visible = false;
			}
			for (int i = 1; i < MAX_CHUNKS - 1; i++) {
			}
			break;
		}

		case(NORTH): {
			for (int i = 0; i < MAX_CHUNKS; i++) {
				for (int j = MAX_CHUNKS-1; j >= 1; j--) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i * MAX_CHUNKS + j].h->vertices[k].y = chunk[i * MAX_CHUNKS + (j-1)].h->vertices[k].y;
					}
				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				//perlinGen(i, 0);
				chunk[MAX_CHUNKS*i].started = chunk[MAX_CHUNKS*i].finished = chunk[MAX_CHUNKS*i].visible = false;
			}
			break;
		}

		case(SOUTH): {
			for (int i = 0; i < MAX_CHUNKS; i++) {
				for (int j = 0; j < MAX_CHUNKS - 1; j++) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i * MAX_CHUNKS + j].h->vertices[k].y = chunk[i * MAX_CHUNKS + (j + 1)].h->vertices[k].y;
					};
				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				chunk[MAX_CHUNKS*i + MAX_CHUNKS - 1].started = chunk[MAX_CHUNKS*i + MAX_CHUNKS - 1].finished = chunk[MAX_CHUNKS*i + MAX_CHUNKS - 1].visible = false;
			}
			break;
		}

	}
	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			chunk[i * MAX_CHUNKS + j].h->BufferData();
		}
	}
}

//Set the pointer for all the chunks
void Renderer::setPointers() {
	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			if (i > 0)					chunk[i * MAX_CHUNKS + j].w = &chunk[(i - 1) * MAX_CHUNKS + j];
			if (i < MAX_CHUNKS - 1)		chunk[i * MAX_CHUNKS + j].e = &chunk[(i + 1) * MAX_CHUNKS + j];
			if (j > 0)					chunk[i * MAX_CHUNKS + j].n = &chunk[i * MAX_CHUNKS + (j - 1)];
			if (j < MAX_CHUNKS - 1)		chunk[i * MAX_CHUNKS + j].s = &chunk[i * MAX_CHUNKS + (j + 1)];
		}
	}
}

void Renderer::threadLoop(int id, unsigned long long c) {
	while (true) {
		for (int i = 0; i < MAX_CHUNKS; i++) {
			for (int j = 0; j < MAX_CHUNKS; j++) {
				if (!chunk[i * MAX_CHUNKS + j].started)	//If a thread hasn't started work on chunk[i][j] yet...
				{
					chunk[i * MAX_CHUNKS + j].started = true; //Signal that this thread has started
					

					for (int x = 0; x < RAW_HEIGHT; x++) {
						for (int y = 0; y < RAW_WIDTH; y++) {
							chunk[i * MAX_CHUNKS + j].h->vertices[x + RAW_WIDTH*y].y =									//Add together multiple noises.
								  generator->simplex(y + (cameraPosX + i)*RAW_WIDTH, x + (cameraPosY + j)*RAW_HEIGHT)
								+ generator->perlin(y + (cameraPosX + i)*RAW_WIDTH, x + (cameraPosY + j)*RAW_HEIGHT)
								+ generator2->perlin(y + (cameraPosX + i)*RAW_WIDTH, x + (cameraPosY + j)*RAW_HEIGHT)
								+ generator3->perlin(y + (cameraPosX + i)*RAW_WIDTH, x + (cameraPosY + j)*RAW_HEIGHT);
						}
					}

					chunk[i * MAX_CHUNKS + j].finished = true;	//Let main program know this chunk has generated

				}
			}
		}
	}
}