//----------------------------------------------------------------------------------------
/**
*      file		|		rain.frag
*	   source	|		07_Textures.pdf
*/
//----------------------------------------------------------------------------------------
#version 140

uniform sampler2D texSampler;  // texture sampler - 
smooth in vec2 texCoord_v;     // texture coordinates
out vec4  color_f;        // output fragment color

void main() 
{	
	vec4 texColor = texture(texSampler, texCoord_v); //sample textures
	color_f = texColor; //barva textury bez svetla
}