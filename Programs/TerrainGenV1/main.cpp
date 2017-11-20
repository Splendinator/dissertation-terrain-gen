#include "../../nclgl/window.h"
#include "Renderer.h"
#include <random>
#include <iostream>

#pragma comment(lib, "nclgl.lib")

int main() {

	int tx = 0;
	int ty = 3;

	UINT id = 0;
	float rad = 30.0f, dy = 0.5f;
	Chunk *prev = NULL;
	Chunk *active = NULL;

	Window w("Index Buffers!", 1080,720,false);
	if(!w.HasInitialised()) {
		return -1;
	}

	w.GetMouse()->SetMouseSensitivity(0.1f);
	

	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}

	
	//w.SetRenderer(&renderer);

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);


	//renderer.perlinGen(0, 0);
	//renderer.perlinGen(0, 1);
	//renderer.perlinGen(0, 2);
	//renderer.perlinGen(1, 0);
	//renderer.perlinGen(1, 1);
	//renderer.perlinGen(1, 2);
	//renderer.perlinGen(2, 0);
	//renderer.perlinGen(2, 1);
	//renderer.perlinGen(2, 2);


	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		renderer.UpdateScene(w.GetTimer()->GetTimedMS());
		renderer.RenderScene();
		prev = active;
		active = renderer.getActiveChunk();


		if (Window::GetMouse()->ButtonDown(MOUSE_LEFT)) {
			active->makeHill(Vector2(fmod((renderer.camera->GetPosition().x / RAW_WIDTH / HEIGHTMAP_X),1.0f),fmod((renderer.camera->GetPosition().z / RAW_WIDTH / HEIGHTMAP_Z),1.0f)), dy, rad,++id);	
		}
		if (Window::GetMouse()->ButtonDown(MOUSE_RIGHT)) {
			active->makeHill(Vector2(fmod((renderer.camera->GetPosition().x / RAW_WIDTH / HEIGHTMAP_X), 1.0f), fmod((renderer.camera->GetPosition().z / RAW_WIDTH / HEIGHTMAP_Z), 1.0f)), -dy, rad,++id);
		}

	}
	return 0;
}

