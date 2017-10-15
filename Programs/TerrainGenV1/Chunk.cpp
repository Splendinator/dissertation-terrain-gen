#include "Chunk.h"



Chunk::Chunk()
{
	Chunk(Vector2(0.0f,0.0f));
	
}

Chunk::Chunk(Vector2 pos) {
	h = new HeightMap(pos);
	wc = pos;
	prevId = 0;

	h->SetTexture(SOIL_load_OGL_texture(
		"../../Textures/BarrenReds.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	
	if (!h->GetTexture()) {
		return;
	}
	
	//TODO: Figure out what the fuck to do with textures later when we want snow on top of mountains and that.
	glBindTexture(GL_TEXTURE_2D, h->GetTexture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, false ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, false ? GL_REPEAT : GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);
}


Chunk::~Chunk()
{
	delete h;
}

void Chunk::Draw() {
	h->Draw();
}

void Chunk::makeHill(Vector2 pos, float dy, float rad, UINT id) {
	if (prevId == id) {
		return;
	}
	prevId = id;

	int x, y, tx = RAW_WIDTH*pos.x, ty = RAW_WIDTH*pos.y;
	int minX = pos.x*RAW_WIDTH - rad, maxX = pos.x*RAW_WIDTH + rad, minY = pos.y*RAW_HEIGHT - rad, maxY = pos.y*RAW_HEIGHT + rad;
	float dist;

	if (minX < 0) {
		minX = 0;
		if (w != NULL)
		w->makeHill(Vector2(pos.x + (1.0f - 1.0f/(RAW_WIDTH) ), pos.y), dy, rad,id); // TODO: Figure out why we need "-1.0f/(RAW_WIDTH)" here but not for east?
	}
	if (minY < 0) {
		minY = 0;
		if (n != NULL)
		n->makeHill(Vector2(pos.x, pos.y + (1.0f - 1.0f/(RAW_HEIGHT))), dy, rad,id);
	}
	if (maxX > RAW_WIDTH) {
		maxX = RAW_WIDTH;
		if(e!=NULL)
		e->makeHill(Vector2(pos.x - 1.0f, pos.y), dy, rad, id);
	}
	if (maxY > RAW_HEIGHT) {
		maxY = RAW_HEIGHT;
		if(s!=NULL)
		s->makeHill(Vector2(pos.x, pos.y - 1.0f), dy, rad, id);
	}

	for (x = minX; x < maxX; x++) {
		for (y = minY; y < maxY; y++) {
			dist = sqrt((tx - x)*(tx - x) + (ty - y)*(ty - y));
			if (dist < rad) {
				//dist = dy*((rad - dist) / rad);
				h->vertices[x*RAW_WIDTH + y].y += sqrt(rad - dist)*dy; // y = sqrt(rad - x^2)		x=dy*((rad-dist)/rad)
			}
		}
	}
	
	h->BufferData();

}

void Chunk::Generate() {

}




