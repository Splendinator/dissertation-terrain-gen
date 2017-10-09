# version 150 core

in Vertex {
vec4 colour ;
} IN ;

vec4 temp;

out vec4 gl_FragColor ;

void main ( void ) {
	temp.x = 0;
	temp.y = 0;
	temp.z = 0;
	temp.w = 1;

	if (IN.colour.x > IN.colour.y && IN.colour.x > IN.colour.z) temp.x = 1;
	else if(IN.colour.y > IN.colour.x && IN.colour.y > IN.colour.z) temp.y = 1;
	else temp.z = 1;

	gl_FragColor = temp ;
 }