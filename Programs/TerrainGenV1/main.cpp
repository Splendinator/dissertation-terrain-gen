#include "../../nclgl/window.h"
#include "Renderer.h"
#include <random>

#pragma comment(lib, "nclgl.lib")

int main() {

	float rad = 30.0f, dy = 0.2f;

	Window w("Index Buffers!", 1600,900,false);
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

	renderer.camera->SetPosition(Vector3(HEIGHTMAP_X / 2 * RAW_WIDTH, HEIGHTMAP_Y*RAW_HEIGHT, HEIGHTMAP_Z / 2 * RAW_WIDTH));

	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		renderer.UpdateScene(w.GetTimer()->GetTimedMS());
		renderer.RenderScene();

		if (Window::GetMouse()->ButtonDown(MOUSE_LEFT)) {
			renderer.heightMap->makeHill(Vector2(renderer.camera->GetPosition().x / RAW_WIDTH / HEIGHTMAP_X, renderer.camera->GetPosition().z / RAW_WIDTH / HEIGHTMAP_Z), dy, rad);
		}
		if (Window::GetMouse()->ButtonDown(MOUSE_RIGHT)) {
			renderer.heightMap->makeHill(Vector2(renderer.camera->GetPosition().x / RAW_WIDTH / HEIGHTMAP_X, renderer.camera->GetPosition().z / RAW_WIDTH / HEIGHTMAP_Z), -dy, rad);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F)) {
			renderer.heightMap->makeFlat();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_R)) {

			std::random_device(e);
			std::uniform_real_distribution<float> rPos(0.0f, 1.0f);
			std::uniform_real_distribution<float> rRad(50.0f, 90.0f);
			std::uniform_real_distribution<float> rSize(-7.0f, 7.0f);

			renderer.heightMap->makeFlat();
			for (int i = 0; i < 400; i++) {
				renderer.heightMap->makeHill(Vector2(rPos(e),rPos(e)),rSize(e),rRad(e));
			}
		}
		if (Window::GetMouse()->GetWheelMovement() > 0){
			rad += 0.4f;
			//dy += 0.002f;
			if (rad < 0) rad = 0;
			if (dy < 0) dy = 0;
		}
		else if (Window::GetMouse()->GetWheelMovement() < 0) {
			rad -= 0.4f;
			//dy -= 0.002f;
			if (rad < 0) rad = 0;
			if (dy < 0) dy = 0;
		}


	}
	return 0;
}

