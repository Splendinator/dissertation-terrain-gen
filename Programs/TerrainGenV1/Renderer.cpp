#include <iostream>
#include <random>
#include "Renderer.h"



UINT id = 0;

Renderer::Renderer(Window & parent) : OGLRenderer(parent) {

	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			chunk[i][j] = new Chunk(Vector2(i*(CHUNK_SIZE),j*(CHUNK_SIZE)));
		}
	}
	setPointers();

	generator = new Generator(-700.0f,700.0f,150.0f);

	camera =			new Camera();
	camera->SetPosition(Vector3(CHUNK_SIZE * MAX_CHUNKS / 2, HEIGHTMAP_Y*RAW_HEIGHT, CHUNK_SIZE * MAX_CHUNKS / 2));
	
	currentShader =		new Shader("../../Shaders/TexturedVertex.glsl",
						"../../Shaders/TexturedFragment.glsl");

	if (!currentShader->LinkProgram()) {
		return;
	}


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
	if (camera->GetPosition().x < 0 + CHUNK_SIZE * (int)(MAX_CHUNKS / 2)) { camera->SetPosition(camera->GetPosition() + Vector3(CHUNK_SIZE, 0.0f, 0.0f));					cameraPosX--;	shiftChunks(WEST);		}
	else if (camera->GetPosition().x > CHUNK_SIZE + CHUNK_SIZE * (int)(MAX_CHUNKS/2)) { camera->SetPosition(camera->GetPosition() + Vector3(-CHUNK_SIZE, 0.0f, 0.0f));		cameraPosX++;	shiftChunks(EAST);		}
	if (camera->GetPosition().z < 0 + CHUNK_SIZE * (int)(MAX_CHUNKS / 2)) { camera->SetPosition(camera->GetPosition() + Vector3(0.0f, 0.0f, CHUNK_SIZE));					cameraPosY--;	shiftChunks(NORTH);		}
	else if (camera->GetPosition().z > CHUNK_SIZE + CHUNK_SIZE * (int)(MAX_CHUNKS / 2)) { camera->SetPosition(camera->GetPosition() + Vector3(0.0f, 0.0f, -CHUNK_SIZE));	cameraPosY++;	shiftChunks(SOUTH);		}
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(currentShader->GetProgram());
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			chunk[i][j]->Draw();
		}
	}
	


	glUseProgram(0);
	SwapBuffers();
}

Chunk * Renderer::getActiveChunk() {
	//float x = camera->GetPosition().x/CHUNK_SIZE, y = camera->GetPosition().z / CHUNK_SIZE;
	//float tx,ty,temp, dist = INFINITE;
	//int ret = 0;
	//
	//
	//for (int i = 0; i < MAX_CHUNKS*MAX_CHUNKS; i++) {
	//	tx = (chunk[i]->getPosition().x + CHUNK_SIZE/2) / CHUNK_SIZE;
	//	ty = (chunk[i]->getPosition().y + CHUNK_SIZE/2) / CHUNK_SIZE;
	//	temp = sqrt((x - tx)*(x - tx) + (y - ty)*(y - ty));
	//	if (dist > temp) {dist = temp; ret = i;}
	//}
	////DrawDebugLine(DEBUGDRAW_ORTHO, camera->GetPosition(), Vector3(chunk[ret]->getPosition().x + CHUNK_SIZE / 2, 0.0f, chunk[ret]->getPosition().y + CHUNK_SIZE / 2));
	////cout << tx << " " << ty << " " << x << " " << y << " " << ret << " " << dist << " " << temp << endl;
	//for (int i = 0; i < MAX_CHUNKS*MAX_CHUNKS; i++) {
	//	cout << chunk[i]->getPosition();
	//}
	//cout << camera->GetPosition() << endl;
	//return chunk[ret]; 

	return chunk[MAX_CHUNKS / 2][MAX_CHUNKS / 2];

}

//TODO: Optimise this?
void Renderer::shiftChunks(Direction dir) {
	int offset;
	switch(dir){
		case(WEST): {
			for (int i = MAX_CHUNKS-1; i >= 1 ; i--) {
				for (int j = 0; j < MAX_CHUNKS; j++) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i][j]->h->vertices[k].y =  chunk[i - 1][j]->h->vertices[k].y;
					}

				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				perlinGen(0,i);
			}
			for (int i = 1; i < MAX_CHUNKS - 1; i++) {
			}
			break;
		}
		 
		case(EAST): {
			for (int i = 0; i < MAX_CHUNKS - 1; i++) {
				for (int j = 0; j < MAX_CHUNKS; j++) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i][j]->h->vertices[k].y = chunk[i + 1][j]->h->vertices[k].y;
					}
				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				perlinGen(MAX_CHUNKS - 1, i);
			}
			for (int i = 1; i < MAX_CHUNKS - 1; i++) {
			}
			break;
		}

		case(NORTH): {
			for (int i = 0; i < MAX_CHUNKS; i++) {
				for (int j = MAX_CHUNKS-1; j >= 1; j--) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i][j]->h->vertices[k].y = chunk[i][j-1]->h->vertices[k].y;
					}
				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				perlinGen(i, 0);
			}
			break;
		}

		case(SOUTH): {
			for (int i = 0; i < MAX_CHUNKS; i++) {
				for (int j = 0; j < MAX_CHUNKS - 1; j++) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i][j]->h->vertices[k].y = chunk[i][j + 1]->h->vertices[k].y;
					};
				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				perlinGen(i,MAX_CHUNKS - 1);
			}
			break;
		}

	}
	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			chunk[i][j]->h->BufferData();
		}
	}
}

//Set the pointer for all the chunks
void Renderer::setPointers() {
	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			if (i > 0) chunk[i][j]->w = chunk[i - 1][j];
			if (i < MAX_CHUNKS - 1) chunk[i][j]->e = chunk[i + 1][j];
			if (j > 0) chunk[i][j]->n = chunk[i][j - 1];
			if (j < MAX_CHUNKS - 1) chunk[i][j]->s = chunk[i][j + 1];
		}
	}
}

void Renderer::perlinGen(const int &x, const int &y) {

	for (int i = 0; i < RAW_HEIGHT; i++) {
		for (int j = 0; j < RAW_WIDTH; j++) {
			chunk[x][y]->h->vertices[j + RAW_WIDTH*i].y = generator->perlin(i + (cameraPosX+x)*RAW_WIDTH, j + (cameraPosY+y)*RAW_HEIGHT);
			//cout << (i + (cameraPosX + x)*RAW_WIDTH) << " | " << (cameraPosY + y)*RAW_HEIGHT << endl;
		}
	}
	chunk[x][y]->h->BufferData();
		
}