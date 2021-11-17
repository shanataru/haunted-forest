//----------------------------------------------------------------------------------------
/**
*      file	|      fs.frag
*/
//----------------------------------------------------------------------------------------
#version 140

// currently used material
struct Material 
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess; // sharpness of specular reflection
    bool useTexture;
};

// light parameters
struct Light 
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec4 position;

    // spotlight direction
    vec3 spotDirection;
    
	// cosine of the spotlight's half angle
    float spotCosCutOff;

    // distribution of the light energy in reflectors cone
    float spotExponent;
};

// sampler for the texture access
uniform sampler2D texSampler;

// current material
uniform Material material;

uniform float time;

// Projection * View * Model  (model to clip coordinates)
uniform mat4 PVMmatrix;

// View                       (world to eye coordinates)
uniform mat4 Vmatrix;

// Model                      (model to world coordinates)
uniform mat4 Mmatrix;

uniform mat4 normalMatrix; // inverse transposed VMmatrix

//fog
uniform float fogDensity;
uniform vec4 fogColor;
uniform bool fogOn;

//sun
Light sun;
uniform bool sunOn;

//point - ghost
Light pointLighter;
uniform vec4 pointlightPosition;
uniform bool pointlightOn;

//flashlight
Light reflector;
uniform vec4 reflectorPosition;
uniform vec3 reflectorDirection;
uniform bool reflectorOn;

smooth in vec2 texCoord_v;	// fragment texture coordinates
smooth in vec3 normal_v;	//camera space normal
smooth in vec3 position_v;	// camera space position
out vec4 color_f;			// outgoing fragment color

// -----------------------------------------------------------------------------------------------------------------------------------------------------

void setupLights()
{
    // set up sun parameters
    sun.ambient = vec3(0.5f);
    sun.diffuse = vec3(0.5f, 0.5f, 0.5f);
    sun.specular = vec3(0.2f);

	// fixed sun position
    sun.position = vec4(12.0f, 12.0f, 1.0f, 0.0f);

    // set up reflector parameters
    reflector.ambient = vec3(0.2f);
    reflector.diffuse = vec3(1.0f);
    reflector.specular = vec3(1.0f);
    reflector.spotCosCutOff = 0.95f;
    reflector.spotExponent = 4.0;

    reflector.position = Vmatrix * reflectorPosition;
    reflector.spotDirection = normalize((Vmatrix * vec4(reflectorDirection, 0.0f)).xyz);

	if (pointlightOn == true) 
	{
        pointLighter.ambient = vec3(0.05f);
		pointLighter.diffuse = vec3(0.3f, 0.0f, 0.6f);
        pointLighter.specular = vec3(0.0f);
        pointLighter.position = (Vmatrix * pointlightPosition);
    }
}

vec4 directionalLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal)
{
    vec3 ret = vec3(0.0);

    vec3 L = normalize(light.position.xyz); //position is vec4
    //svetlo jako vektor, smer sviceni slunce
	vec3 R = reflect(-L, vertexNormal);
    vec3 V = normalize(-vertexPosition);
    vec3 diffuse_ref = max(0.0f, dot(vertexNormal, L)) * material.diffuse * light.diffuse;
    vec3 ambient_ref = material.ambient * light.ambient;
    vec3 specular_ref = pow(max(0.0f, dot(R, V)), material.shininess) * material.specular * light.specular;
   
    ret += (diffuse_ref + ambient_ref + specular_ref);

    return vec4(ret, 1.0f);
}

vec4 pointLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal)
{
    vec3 ret = vec3(0.0f);
	vec3 L = normalize(light.position.xyz - vertexPosition); // - vertexPosition
    vec3 R = reflect(-L, vertexNormal);
    vec3 V = normalize(-vertexPosition);
	vec3 diffuse_ref = max(0.0f, dot(vertexNormal, L)) * material.diffuse * light.diffuse;
	vec3 ambient_ref = material.ambient * light.ambient;
	vec3 specular_ref = pow(max(0.0f, dot(R, V)), material.shininess) * material.specular * light.specular;

	vec3 att = vec3(0.0f, 0.0f, 1.5f);
	float dist = length(light.position.xyz - vertexPosition);
	float attFact = 1.0f / (att.x + att.y * dist + att.z * dist * dist);
    ret = attFact * (diffuse_ref + ambient_ref + specular_ref);
    return vec4(ret, 1.0f);
}

vec4 spotLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal)
{
    vec3 ret = vec3(0.0);
    vec3 L = normalize(light.position.xyz - vertexPosition);
    //potreba ziskat vektor smeru svetla (jednotkovy)
	vec3 R = reflect(-L, vertexNormal);
    vec3 V = normalize(-vertexPosition);
    vec3 diffuse_ref = max(0.0f, dot(vertexNormal, L)) * material.diffuse * light.diffuse;
    vec3 ambient_ref = material.ambient * light.ambient;
    vec3 specular_ref = pow(max(0.0f, dot(R, V)), material.shininess) * material.specular * light.specular;
    ret += (diffuse_ref + ambient_ref + specular_ref);
    float koeficient = max(0.0f, dot(-L, light.spotDirection));
    //uhel!!

	if (koeficient >= light.spotCosCutOff) //cosinus!
	  ret *= pow(koeficient, light.spotExponent);
    else
	  ret *= 0.0f;
    return vec4(ret, 1.0);
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------

void main()
{
    setupLights();
    
	// initialize the output color with the global ambient term
    vec3 globalAmbientLight = vec3(0.2f);
    vec4 outputColor = vec4(material.ambient * globalAmbientLight, 0.0f);
    
	//sun
    if (sunOn)
        outputColor += directionalLight(sun, material, position_v, normal_v);
    
	//reflector
    if (reflectorOn)
        outputColor += spotLight(reflector, material, position_v, normal_v);
    
	//ghost
    if (pointlightOn)
        outputColor += pointLight(pointLighter, material, position_v, normal_v);
	
	//assign color depending on which light is used
	color_f = outputColor;

	// texture - modulate object color by the texture
    if (material.useTexture)
        color_f = outputColor * texture(texSampler, texCoord_v);
   
   	//fog - source: 08_Misc.pdf
    if (fogOn) 
	{
        float fogMode = 0.0;
        fogMode = exp(-pow(fogDensity * abs(gl_FragCoord.z / gl_FragCoord.w), 2.0f));
        fogMode = 1.0f - clamp(fogMode, 0.0f, 1.0f);
        color_f = mix(color_f, fogColor, fogMode);
    }
}