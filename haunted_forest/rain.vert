//----------------------------------------------------------------------------------------
/**
*      file		|		rain.vert
*	   source	|		07_Textures.pdf
*/
//----------------------------------------------------------------------------------------
#version 140

uniform mat4 PVMmatrix; 
uniform mat4 texTransMatrix;
in vec3 position; // input vertex position
in vec2 texCoord; // intput vertex texture coordinates
smooth out vec2 texCoord_v; //output vertex texture coordinates

void main() 
{
	gl_Position = PVMmatrix * vec4(position, 1);   
	vec4 transCoord = texTransMatrix * vec4(texCoord, 1.0f , 1.0f);
	texCoord_v = transCoord.xy;
}
