#include "Renderer.h"
#include "Chunk.h"
#include <iostream>


Renderer::Renderer(Window & parent) : OGLRenderer(parent) {

	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			chunk[i*MAX_CHUNKS+j] = new Chunk(Vector2(i*(CHUNK_SIZE),j*(CHUNK_SIZE)));
		}
	}


	for (int i = 0; i < MAX_CHUNKS; i++) {
		for (int j = 0; j < MAX_CHUNKS; j++) {
			if (i > 0)						chunk[i * 3 + j]->w = chunk[(i - 1) * 3 + j];
			if (j > 0)						chunk[i * 3 + j]->n = chunk[i * 3 + (j-1)];
			if (i < MAX_CHUNKS-1)			chunk[i * 3 + j]->e = chunk[(i + 1) * 3 + j];
			if (j < MAX_CHUNKS-1)			chunk[i * 3 + j]->s = chunk[i * 3 + (j + 1)];

		}
	}

	camera =			new Camera();
	
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
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(currentShader->GetProgram());
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);

	for (int i = 0; i < MAX_CHUNKS*MAX_CHUNKS; i++) {
		chunk[i]->Draw();
	}
	

	glUseProgram(0);
	SwapBuffers();
}

Chunk * Renderer::getActiveChunk() {
	float x = camera->GetPosition().x/CHUNK_SIZE, y = camera->GetPosition().z / CHUNK_SIZE;
	float tx,ty,temp, dist = INFINITE;
	int ret = 0;

	for (int i = 0; i < MAX_CHUNKS*MAX_CHUNKS; i++) {
		tx = (chunk[i]->getPosition().x + CHUNK_SIZE/2) / CHUNK_SIZE;
		ty = (chunk[i]->getPosition().y + CHUNK_SIZE/2) / CHUNK_SIZE;
		temp = sqrt((x - tx)*(x - tx) + (y - ty)*(y - ty));
		if (dist > temp) {dist = temp; ret = i;}
	}
	//DrawDebugLine(DEBUGDRAW_ORTHO, camera->GetPosition(), Vector3(chunk[ret]->getPosition().x + CHUNK_SIZE / 2, 0.0f, chunk[ret]->getPosition().y + CHUNK_SIZE / 2));
	//cout << tx << " " << ty << " " << x << " " << y << " " << ret << " " << dist << " " << temp << endl;
	return chunk[ret]; 
}

void Renderer::shiftChunks(Direction dir) {
	if (dir == NORTH) {
		for (int i = 0; i < MAX_CHUNKS*MAX_CHUNKS; i++) {
			if (chunk[i]->s == NULL) {
				chunk[i]->n->s = NULL;	//Delete the pointer to this chunk...
				//delete chunk[i];
				chunk[i] = new Chunk();
			}
		}
	}
}