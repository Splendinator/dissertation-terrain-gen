#version 430
uniform sampler2D grassTex;
uniform sampler2D snowTex;
uniform sampler2D rockTex;
uniform sampler2D sandTex;

in Vertex {
	vec2 texCoord;
	float height;
	vec2 gradient;
	vec4 biomes;
} IN;

out vec4 gl_FragColor;


const float SNOW_HEIGHT = 400f;
const float SNOW_LERP = 400f;

const float ROCK_GRAD = 5f;
const float ROCK_LERP = 12f;

vec4 lerp(sampler2D t1, sampler2D t2, float pct){
		if (pct > 1) pct = 1;
		if (pct < 0) pct = 0;
		return texture(t1,IN.texCoord) * (1-pct) + texture(t2,IN.texCoord) * pct;
}

vec4 lerp(vec4 t1, sampler2D t2, float pct){
		if (pct > 1) pct = 1;
		if (pct < 0) pct = 0;
		return t1 * (1-pct) + texture(t2,IN.texCoord) * pct;
}

void main(void){
	gl_FragColor = texture(grassTex, IN.texCoord);
	if (IN.height > SNOW_HEIGHT) { gl_FragColor = lerp(gl_FragColor,snowTex,(IN.height-SNOW_HEIGHT) / SNOW_LERP); }
	

	float grad = sqrt(IN.gradient.x * IN.gradient.x + IN.gradient.y * IN.gradient.y); //Get magnitude of gradient
	if (grad > ROCK_GRAD) { gl_FragColor = lerp(gl_FragColor,rockTex,((grad-ROCK_GRAD)  / ROCK_LERP) * IN.biomes.y); }

	gl_FragColor = lerp(gl_FragColor,sandTex,IN.biomes.z);
}


