#version 430
uniform sampler2D grassTex;
uniform sampler2D snowTex;

in Vertex {
	vec2 texCoord;
	float height;
} IN;

out vec4 gl_FragColor;


const float SNOW_HEIGHT = 400f;
const float SNOW_LERP = 400f;

vec4 lerp(sampler2D t1, sampler2D t2, float pct){
		if (pct > 1) pct = 1;
		if (pct < 0) pct = 0;
		return texture(t1,IN.texCoord) * (1-pct) + texture(t2,IN.texCoord) * pct;
}

void main(void){
	gl_FragColor = texture(grassTex, IN.texCoord);
	if (IN.height > SNOW_HEIGHT) { gl_FragColor = lerp(grassTex,snowTex,(IN.height-SNOW_HEIGHT) / SNOW_LERP); }
}


