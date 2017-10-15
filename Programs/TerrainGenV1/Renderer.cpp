#include "Renderer.h"
#include "Chunk.h"
#include <iostream>
#include <random>


UINT id = 0;

Renderer::Renderer(Window & parent) : OGLRenderer(parent) {

	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			chunk[i][j] = new Chunk(Vector2(i*(CHUNK_SIZE),j*(CHUNK_SIZE)));
		}
	}
	setPointers();
	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			cout << "[" << i << "][" << j << "]" << endl;
			if (chunk[i][j]->n != NULL) cout << "\t N" << endl;
			if (chunk[i][j]->e != NULL) cout << "\t E" << endl;
			if (chunk[i][j]->s != NULL) cout << "\t S" << endl;
			if (chunk[i][j]->w != NULL) cout << "\t W" << endl;

		}
	}

	

	camera =			new Camera();
	camera->SetPosition(Vector3((HEIGHTMAP_X / 2 * RAW_WIDTH) + (HEIGHTMAP_X*RAW_WIDTH)*MAX_CHUNKS/2, HEIGHTMAP_Y*RAW_HEIGHT, (HEIGHTMAP_Z / 2 * RAW_WIDTH) + (HEIGHTMAP_Z*RAW_HEIGHT))*MAX_CHUNKS / 2);
	
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
	if (camera->GetPosition().x < 0 + CHUNK_SIZE * (int)(MAX_CHUNKS / 2)) { camera->SetPosition(camera->GetPosition() + Vector3(CHUNK_SIZE, 0.0f, 0.0f));					shiftChunks(WEST);		}
	else if (camera->GetPosition().x > CHUNK_SIZE + CHUNK_SIZE * (int)(MAX_CHUNKS/2)) { camera->SetPosition(camera->GetPosition() + Vector3(-CHUNK_SIZE, 0.0f, 0.0f));		shiftChunks(EAST);		}
	if (camera->GetPosition().z < 0 + CHUNK_SIZE * (int)(MAX_CHUNKS / 2)) { camera->SetPosition(camera->GetPosition() + Vector3(0.0f, 0.0f, CHUNK_SIZE));					shiftChunks(NORTH);		}
	else if (camera->GetPosition().z > CHUNK_SIZE + CHUNK_SIZE * (int)(MAX_CHUNKS / 2)) { camera->SetPosition(camera->GetPosition() + Vector3(0.0f, 0.0f, -CHUNK_SIZE));	shiftChunks(SOUTH);		}
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
	//********* random delete this *************
	std::random_device(e);
	std::uniform_real_distribution<float> rPos(0.0f, 1.0f);
	std::uniform_real_distribution<float> rRad(100.0f, 150.0f);
	std::uniform_real_distribution<float> rSize(10.0f, 14.0f);
	//********* random delete this *************
	switch(dir){
		case(WEST): {
			for (int i = MAX_CHUNKS-1; i >= 1 ; i--) {
				for (int j = 0; j < MAX_CHUNKS; j++) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i][j]->h->vertices[k].y =  chunk[i - 1][j]->h->vertices[k].y;
					}

					chunk[i][j]->h->BufferData();
				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				
				
				/*TEMP RANDOM HILLS*/
				for (int j = 0; j < 6; j++) {
					chunk[0][i]->makeHill(Vector2(rPos(e), rPos(e)), rSize(e), rRad(e), ++id);
				}
				/*TEMP RANDOM HILLS*/

				chunk[0][i]->h->BufferData();
			}
			break;
		}
		 
		case(EAST): {
			for (int i = 0; i < MAX_CHUNKS - 1; i++) {
				for (int j = 0; j < MAX_CHUNKS; j++) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i][j]->h->vertices[k].y = chunk[i + 1][j]->h->vertices[k].y;
					}
					chunk[i][j]->h->BufferData();
				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				chunk[MAX_CHUNKS-1][i]->h->makeFlat();
				chunk[MAX_CHUNKS-1][i]->h->BufferData();
			}
			break;
		}

		case(NORTH): {
			for (int i = 0; i < MAX_CHUNKS; i++) {
				for (int j = MAX_CHUNKS-1; j >= 1; j--) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i][j]->h->vertices[k].y = chunk[i][j-1]->h->vertices[k].y;
					}
					chunk[i][j]->h->BufferData();
				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				chunk[i][0]->h->makeFlat();
				chunk[i][0]->h->BufferData();
			}
			break;
		}

		case(SOUTH): {
			for (int i = 0; i < MAX_CHUNKS; i++) {
				for (int j = 0; j < MAX_CHUNKS - 1; j++) {
					for (int k = 0; k < RAW_HEIGHT*RAW_WIDTH; k++) {
						chunk[i][j]->h->vertices[k].y = chunk[i][j + 1]->h->vertices[k].y;
					}
					chunk[i][j]->h->BufferData();
				}
			}
			for (int i = 0; i < MAX_CHUNKS; i++) {
				chunk[i][MAX_CHUNKS - 1]->h->makeFlat();
				chunk[i][MAX_CHUNKS - 1]->h->BufferData();
			}
			break;
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