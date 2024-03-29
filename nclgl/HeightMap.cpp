#include "HeightMap.h"

HeightMap::HeightMap() {

	numVertices = RAW_WIDTH * RAW_HEIGHT;
	numIndices = (RAW_WIDTH - 1)*(RAW_HEIGHT - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];
	shadePct = new float[numVertices];
	water = new float[numVertices];
	texturePct = new Vector4[numVertices];


	for (int x = 0; x < RAW_WIDTH; ++x) {
		for (int z = 0; z < RAW_HEIGHT; ++z) {
			int offset = (x * RAW_WIDTH) + z;

			vertices[offset] = Vector3(
				x * HEIGHTMAP_X, 0.0f, z * HEIGHTMAP_Z);

			textureCoords[offset] = Vector2(
				x * HEIGHTMAP_TEX_X, z * HEIGHTMAP_TEX_Z);
		}
	}


	numIndices = 0;

	for (int x = 0; x < RAW_WIDTH - 1; ++x) {
		for (int z = 0; z < RAW_HEIGHT - 1; ++z) {
			int a = (x * (RAW_WIDTH)) + z;
			int b = ((x + 1) * (RAW_WIDTH)) + z;
			int c = ((x + 1) * (RAW_WIDTH)) + (z + 1);
			int d = (x * (RAW_WIDTH)) + (z + 1);

			indices[numIndices++] = c;
			indices[numIndices++] = b;
			indices[numIndices++] = a;

			indices[numIndices++] = a;
			indices[numIndices++] = d;
			indices[numIndices++] = c;

		}
	}

	BufferData();

}



HeightMap::HeightMap(Vector2 pos) {

	

	numVertices = RAW_WIDTH * RAW_HEIGHT;
	numIndices = (RAW_WIDTH - 1)*(RAW_HEIGHT - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];


	for (int x = 0; x < RAW_WIDTH; ++x) {
		for (int z = 0; z < RAW_HEIGHT; ++z) {
			int offset = (x * RAW_WIDTH) + z;
	
			vertices[offset] = Vector3(
				x * HEIGHTMAP_X + pos.x, 0.0f, z * HEIGHTMAP_Z + pos.y);
	
			textureCoords[offset] = Vector2(
				x * HEIGHTMAP_TEX_X, z * HEIGHTMAP_TEX_Z);
		}
	}


	numIndices = 0;

	for (int x = 0; x < RAW_WIDTH - 1; ++x) {
		for (int z = 0; z < RAW_HEIGHT - 1; ++z) {
			int a = (x * (RAW_WIDTH)) + z;
			int b = ((x + 1) * (RAW_WIDTH)) + z;
			int c = ((x + 1) * (RAW_WIDTH)) + (z + 1);
			int d = (x * (RAW_WIDTH)) + (z + 1);

			indices[numIndices++] = c;
			indices[numIndices++] = b;
			indices[numIndices++] = a;

			indices[numIndices++] = a;
			indices[numIndices++] = d;
			indices[numIndices++] = c;

		}
	}

	BufferData();

}



void HeightMap::makeFlat() {
	for (int x = 0; x < RAW_WIDTH; x++) {
		for (int y = 0; y < RAW_WIDTH; y++) {
			vertices[x*RAW_WIDTH + y] = Vector3(vertices[x*RAW_WIDTH + y].x, 0.0f, vertices[x*RAW_WIDTH + y].z);
		}
	}
}

void HeightMap::Draw(){

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureGrass);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureSnow);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureRock);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textureSand);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, textureWater);



	glBindVertexArray(arrayObject);


	if (bufferObject[INDEX_BUFFER]) {
		glDrawElements(type, numIndices, GL_UNSIGNED_INT, 0);
	}
	else {
		glDrawArrays(type, 0, numVertices);
	}

	glBindVertexArray(0);

}


void HeightMap::BufferData() {
	glDeleteBuffers(MAX_BUFFER, bufferObject);
	glBindVertexArray(arrayObject);
	glGenBuffers(1, &bufferObject[VERTEX_BUFFER]);
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[VERTEX_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3),
		vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(VERTEX_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_BUFFER);

	if (textureCoords) { // This bit is new !
		glGenBuffers(1, &bufferObject[TEXTURE_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[TEXTURE_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector2),
			textureCoords, GL_STATIC_DRAW);
		glVertexAttribPointer(TEXTURE_BUFFER, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(TEXTURE_BUFFER);
	}
	if (colours) {
		glGenBuffers(1, &bufferObject[COLOUR_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[COLOUR_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector4),
			colours, GL_STATIC_DRAW);
		glVertexAttribPointer(COLOUR_BUFFER, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(COLOUR_BUFFER);
	}
	if (indices) {
		glGenBuffers(1, &bufferObject[INDEX_BUFFER]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
			bufferObject[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint),
			indices, GL_STATIC_DRAW);
	}
	if (water) {
		glGenBuffers(1, &waterBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, waterBufferObject);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float),
			water, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(6);
	}
	if (texturePct) {
		glGenBuffers(1, &textureBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, textureBufferObject);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector4),
			texturePct, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(7);
	}
	if (shadePct) {
		glGenBuffers(1, &shadeBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, shadeBufferObject);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float),
			shadePct, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(8);
	}

	glBindVertexArray(0);
}



void HeightMap::operator=(HeightMap &rhs) {
	arrayObject = rhs.arrayObject;
	numVertices = rhs.numVertices;
	type = rhs.type;
	texture = rhs.texture;
	numIndices = rhs.numIndices;;

	for (int i = 0; i < MAX_BUFFER; i++) {
		bufferObject[i] = rhs.bufferObject[i];
	}

	if (rhs.textureCoords) {
		textureCoords = new Vector2[numVertices];
		memcpy(textureCoords, rhs.textureCoords, numVertices * sizeof(Vector2));
	}

	if (rhs.vertices) {
		vertices = new Vector3[numVertices];
		memcpy(vertices, rhs.vertices, numVertices * sizeof(Vector3));
	}

	if (rhs.colours) {
		colours = new Vector4[numVertices];
		memcpy(colours, rhs.colours, numVertices * sizeof(Vector4));
	}

	if (rhs.indices) {
		indices = new unsigned int[numIndices];
		memcpy(indices, rhs.indices, numIndices * sizeof(unsigned int));
	}
	textureSnow = rhs.textureSnow;
	textureGrass = rhs.textureGrass;
	textureRock = rhs.textureRock;
	textureSand = rhs.textureSand;
	textureWater = rhs.textureWater;
}

