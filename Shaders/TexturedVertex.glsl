#version 430

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

layout(location = 10) uniform vec2 wc;

in vec3 position;
in vec2 texCoord;
layout(location = 6) in float water;
layout(location = 7) in vec4 texture;
layout(location = 8) in float shade;

out Vertex {
	vec2 texCoord;
	float water;
	vec4 texture;
	float shade;
} OUT;

void main(void) {
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * (vec4(position, 1.0) + vec4(wc.x, 0, wc.y, 0));
	OUT.water = water;
	OUT.texture = texture;
	OUT.shade = shade;
	OUT.texCoord = (textureMatrix * vec4(texCoord, 0.0, 1.0)).xy;
}