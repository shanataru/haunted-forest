//----------------------------------------------------------------------------------------
/**
*      file		|		smoke.frag
*	   source	|		asteroids - explosion.frag
*/
//----------------------------------------------------------------------------------------
#version 140

uniform float time;
uniform mat4 Vmatrix;         // view (camera) transform
uniform sampler2D texSampler; // sampler for texture access

smooth in vec3 position_v;    // camera space fragment position
smooth in vec2 texCoord_v;    // fragment texture coordinates

out vec4 color_f;             // outgoing fragment color

// there are 8 frames in the row, two rows total
uniform ivec2 pattern = ivec2(8, 2);
// one frame lasts 0.1s
uniform float frameDuration = 0.1f;

vec4 sampleTexture(int frame) {
	vec2 offset = vec2(1.0f) / vec2(pattern);					// 1/2 a 1/8 - uniform pattern
	vec2 texStart = texCoord_v / vec2(pattern);					//vypocet v 0.0-1.0
	vec2 relCoord = vec2(frame, (frame / pattern.x)) * offset;	//relativni posun v 0.0-1.0f //0-7 a zpet na 0
	vec2 texCoord = texStart + relCoord;						//frame je 0-15, vykreslim prislusny frame (v cele texture)
	return texture(texSampler, texCoord);						//vraci barvu mista
}

void main() {
  // frame of the texture to be used for smoke
  int frame = int(time / frameDuration);

  // sample proper frame of the texture to get a fragment color  
  color_f = sampleTexture(frame);
}
