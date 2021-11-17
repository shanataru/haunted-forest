//----------------------------------------------------------------------------------------
/**
*      file		|		skybox.frag
*	   source	|		asteriods - data.h
*/
//----------------------------------------------------------------------------------------
#version 140

uniform samplerCube skyboxSampler;
uniform bool fogOn;
//uniform float fogDensity;
//uniform vec4 fogColor;

in vec3 texCoord_v;
out vec4 color_f;

float fogDensity = 2.0f;
vec4 fogColor = vec4(0.53f, 0.13f, 0.13f, 1.0f);

void main()
{
	color_f = texture(skyboxSampler, texCoord_v);

	//source: 08_Misc.pdf
    if (fogOn) {
        float fogMode = 0.0;
        fogMode = exp(-pow(fogDensity * abs(gl_FragCoord.z / gl_FragCoord.w), 2.0f));
        fogMode = 1.0f - clamp(fogMode, 0.0f, 1.0f);
        color_f = mix(color_f, fogColor, fogMode);
    }
}