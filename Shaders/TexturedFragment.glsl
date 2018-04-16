#version 430
uniform sampler2D grassTex;
uniform sampler2D snowTex;
uniform sampler2D rockTex;
uniform sampler2D sandTex;
uniform sampler2D waterTex;

uniform float time;

in Vertex {
	vec2 texCoord;
	float water;
	vec4 texturePct;
	float shade;
} IN;

out vec4 gl_FragColor;

const float WATER_SPEED = 2000;

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

vec4 lerp(vec4 t1, vec4 t2, float pct){
		if (pct > 1) pct = 1;
		if (pct < 0) pct = 0;
		return t1 * (1-pct) + t2 * pct;
}

void main(void){
	
	if(IN.water > 0) { 
		gl_FragColor = texture(waterTex, IN.texCoord + vec2(time/WATER_SPEED,0));
	}
	else {
		//gl_FragColor = lerp(
		//					lerp(grassTex,sandTex,IN.texturePct.x / (IN.texturePct.x+IN.texturePct.y)),
		//					lerp(rockTex,snowTex,IN.texturePct.z / (IN.texturePct.z+IN.texturePct.w)),
		//						(IN.texturePct.x+IN.texturePct.y) / (IN.texturePct.x+IN.texturePct.y + IN.texturePct.z + IN.texturePct.w));
		
		
		gl_FragColor = texture(grassTex,IN.texCoord);
		if(IN.texturePct.y > 0) gl_FragColor = lerp(gl_FragColor,sandTex,IN.texturePct.y / (IN.texturePct.x + IN.texturePct.y));
		if(IN.texturePct.z > 0) gl_FragColor = lerp(gl_FragColor,snowTex,IN.texturePct.z / (IN.texturePct.x + IN.texturePct.y + IN.texturePct.z));
		if(IN.texturePct.w > 0) gl_FragColor = lerp(gl_FragColor,rockTex,IN.texturePct.w / (IN.texturePct.x + IN.texturePct.y + IN.texturePct.z + IN.texturePct.w));

	}

	


	gl_FragColor = lerp(gl_FragColor,vec4(0,0,0,1),IN.shade);

	
}


