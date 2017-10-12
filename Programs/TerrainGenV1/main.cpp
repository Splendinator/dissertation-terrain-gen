#include "../../nclgl/window.h"
#include "Renderer.h"
#include <random>

#pragma comment(lib, "nclgl.lib")

int main() {

	UINT id = 0;
	float rad = 30.0f, dy = 0.5f;
	Chunk *active;

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
		active = renderer.getActiveChunk();

		if (Window::GetMouse()->ButtonDown(MOUSE_LEFT)) {
			active->makeHill(Vector2(fmod((renderer.camera->GetPosition().x / RAW_WIDTH / HEIGHTMAP_X),1.0f),fmod((renderer.camera->GetPosition().z / RAW_WIDTH / HEIGHTMAP_Z),1.0f)), dy, rad,++id);	
		}
		if (Window::GetMouse()->ButtonDown(MOUSE_RIGHT)) {
			active->makeHill(Vector2(fmod((renderer.camera->GetPosition().x / RAW_WIDTH / HEIGHTMAP_X), 1.0f), fmod((renderer.camera->GetPosition().z / RAW_WIDTH / HEIGHTMAP_Z), 1.0f)), -dy, rad,++id);
		}
		//if (Window::GetKeyboard()->KeyDown(KEYBOARD_F)) {
		//	renderer.heightMap->makeFlat();
		//	renderer.heightMap->BufferData();
		//}

		//if (Window::GetKeyboard()->KeyDown(KEYBOARD_1)) {	//Random Mountains

		//	std::random_device(e);
		//	std::uniform_real_distribution<float> rPos(0.0f, 1.0f);
		//	std::uniform_real_distribution<float> rRad(50.0f, 110.0f);
		//	std::uniform_real_distribution<float> rSize(-6.0f, 6.0f);

		//	renderer.heightMap->makeFlat();
		//	for (int i = 0; i < 500; i++) {
		//		renderer.heightMap->makeHill(Vector2(rPos(e),rPos(e)),rSize(e),rRad(e));
		//	}
		//	renderer.heightMap->BufferData();
		//}

		//if (Window::GetKeyboard()->KeyDown(KEYBOARD_2)) {	//Random Spikey Mountains

		//	std::random_device(e);
		//	std::uniform_real_distribution<float> rPos(0.0f, 1.0f);
		//	std::uniform_real_distribution<float> rRad(15.0f, 30.0f);
		//	std::uniform_real_distribution<float> rSize(-10.0f, 10.0f);

		//	renderer.heightMap->makeFlat();
		//	for (int i = 0; i < 1000; i++) {
		//		renderer.heightMap->makeHill(Vector2(rPos(e), rPos(e)), rSize(e), rRad(e));
		//	}
		//	renderer.heightMap->BufferData();
		//}

		//if (Window::GetKeyboard()->KeyDown(KEYBOARD_3)) {	//Flat Field

		//	std::random_device(e);
		//	std::uniform_real_distribution<float> rPos(0.0f, 1.0f);
		//	std::uniform_real_distribution<float> rRad(150.0f, 300.0f);
		//	std::uniform_real_distribution<float> rSize(-3.0f, 3.0f);

		//	renderer.heightMap->makeFlat();
		//	for (int i = 0; i < 20; i++) {
		//		renderer.heightMap->makeHill(Vector2(rPos(e), rPos(e)), rSize(e), rRad(e));
		//	}
		//	renderer.heightMap->BufferData();
		//}

		//if (Window::GetKeyboard()->KeyDown(KEYBOARD_4)) {	//Rocky Field with spikes in

		//	std::random_device(e);
		//	std::uniform_real_distribution<float> rPos(0.0f, 1.0f);
		//	std::uniform_real_distribution<float> rRad(100.0f, 200.0f);
		//	std::uniform_real_distribution<float> rSize(-8.0f, 8.0f);

		//	renderer.heightMap->makeFlat();
		//	for (int i = 0; i <30; i++) {
		//		renderer.heightMap->makeHill(Vector2(rPos(e), rPos(e)), rSize(e), rRad(e));
		//	}

		//	//spikes
		//	rPos = std::uniform_real_distribution<float>(0.0f, 1.0f);
		//	rRad = std::uniform_real_distribution<float>(1.0f, 2.0f);
		//	rSize = std::uniform_real_distribution<float>(30.0f, 80.0f);

		//	for (int i = 0; i < 15; i++) {
		//		renderer.heightMap->makeHill(Vector2(rPos(e), rPos(e)), rSize(e), rRad(e));
		//	}


		//	renderer.heightMap->BufferData();
		//}

		//if (Window::GetKeyboard()->KeyDown(KEYBOARD_5)) {	//Crater

		//	std::random_device(e);
		//	std::uniform_real_distribution<float> rPos(0.2f, 0.8f);
		//	std::uniform_real_distribution<float> rRad(40.0f, 80.0f);
		//	std::uniform_real_distribution<float> rSize(-10.0f, 3.0f);

		//	renderer.heightMap->makeFlat();
		//	for (int i = 0; i < 80; i++) {
		//		renderer.heightMap->makeHill(Vector2(rPos(e), rPos(e)), rSize(e), rRad(e));
		//	}
		//	renderer.heightMap->BufferData();
		//}


		//if (Window::GetMouse()->GetWheelMovement() > 0){
		//	rad += 0.8f;
		//	//dy += 0.002f;
		//	if (rad < 0) rad = 0;
		//	if (dy < 0) dy = 0;
		//}
		//else if (Window::GetMouse()->GetWheelMovement() < 0) {
		//	rad -= 0.8f;
		//	//dy -= 0.002f;
		//	if (rad < 0) rad = 0;
		//	if (dy < 0) dy = 0;
		//}


	}
	return 0;
}

