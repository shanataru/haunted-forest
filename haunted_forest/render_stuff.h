//----------------------------------------------------------------------------------------
/**
*      file	|		render_stuff.h
*/
//----------------------------------------------------------------------------------------
#ifndef __RENDER_STUFF_H
#define __RENDER_STUFF_H

typedef struct MeshGeometry {
	GLuint vertexBufferObject;
	GLuint elementBufferObject;
	GLuint vertexArrayObject;
	unsigned int numTriangles;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;
	GLuint texture;
} MeshGeometry;

typedef struct CameraObject {
	glm::vec3 position;
	glm::vec3 direction;
	float startTime;
	float currentTime;
	float viewAngle;
} CameraObject;

typedef struct Object //tree, skull, mushroom
{
	glm::vec3 position;
	glm::vec3 direction;
	float size;
} Object;

typedef struct GroundObject{
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 size;
	float viewAngle;
} GroundObject;

typedef struct  RainObject {
	glm::vec3 position;
	glm::vec3 direction;
	float startTime;
	float currentTime;
	float size;
} RainObject;

typedef struct FogObject {
	glm::vec4 color;
	float density;
	bool fogOn;
}FogObject;

typedef struct MovingObject {
	glm::vec3 position;
	glm::vec3 initPosition;
	glm::vec3 direction;
	//glm::vec3 directionForReflector;
	float speed;
	float size;
	float startTime;
	float currentTime;
	float viewAngle;
} MovingObject;

typedef struct SmokeObject : RainObject
{
	float speed;
	int texFrames;
	float frameDuration;
	bool destroyed;
} SmokeObject;

typedef struct skyboxShaderProgram {
	GLuint program;
	// vertex attributes locations
	GLint screenCoordLocation;
	GLint inversePVmatrixLocation;
	GLint skyboxSamplerLocation;
	GLint fogOnLocation;
	GLint fogColorLocation;
	GLint fogDensityLocation;
} SSkyboxShaderProgram;

typedef struct rainShaderProgram {
	GLuint program;
	GLint posLocation;
	GLint texCoordLocation;
	GLint PVMmatrixLocation;
	GLint timeLocation;
	GLint texSamplerLocation;
	GLint texTransMatrixLocation;
} SRainShaderProgram;

typedef struct smokeShaderProgram
{
	GLuint program;
	GLint posLocation;
	GLint texCoordLocation;
	GLint PVMmatrixLocation;
	GLint VmatrixLocation;
	GLint timeLocation;
	GLint texSamplerLocation;
	GLint frameDurationLocation;
} SSmokeShaderProgram;

typedef struct _commonShaderProgram {
	GLuint program;
	GLint posLocation;
	GLint colorLocation;
	GLint normalLocation;
	GLint texCoordLocation;

	GLint PVMmatrixLocation;
	// view/camera matrix
	GLint VmatrixLocation;
	//modeling matrix
	GLint MmatrixLocation;
	//inverse transposed VMmatrix
	GLint normalMatrixLocation;
	//elapsed time in seconds
	GLint timeLocation;

	// material
	GLint diffuseLocation;
	GLint ambientLocation;
	GLint specularLocation;
	GLint shininessLocation;
	// texture
	GLint useTextureLocation;
	GLint texSamplerLocation;

	//fog
	GLint fogColorLocation;
	GLint fogDensityLocation;
	GLint fogOnLocation;

	//sun
	GLint sunOnLocation;

	//reflector
	GLint reflectorOnLocation;
	GLint reflectorPositionLocation;
	GLint reflectorDirectionLocation;

	//pointlight
	GLint pointlightPositionLocation;
	GLint pointlightOnLocation;

} SCommonShaderProgram;

// -----------------------------------------------------------------------------------------------------------------------------------------------------

bool loadSingleMesh(const std::string& fileName, SCommonShaderProgram& shader, MeshGeometry** geometry);
void setTransformUniforms(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void setMaterialUniforms(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, GLuint texture);

// -----------------------------------------------------------------------------------------------------------------------------------------------------

void initializeShaderPrograms();
void initgroundMeshGeometry(SCommonShaderProgram& shader, MeshGeometry** geometry);
void initRainGeometry(GLuint shader, MeshGeometry **geometry);
void initSmokeGeometry(GLuint shader, MeshGeometry**geometry);
void initrockMeshGeometry(SCommonShaderProgram& shader, MeshGeometry** geometry);
void initskyboxMeshGeometry(GLuint shader, MeshGeometry** geometry, bool day);
void initializeModels();

// -----------------------------------------------------------------------------------------------------------------------------------------------------

void drawGround(GroundObject* ground, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawRock(Object* rock, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawMeshGeometry(MeshGeometry* geometry, glm::vec3 position, glm::vec3 direction, float size, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawTree(Object* tree, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, int type);
void drawExtra(Object* extra, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, bool diffColor);
void drawSkull(Object* skull, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawMushroom(Object* mush, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawBat(MovingObject* bat, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawGhost(MovingObject* ghost, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawSmoke(SmokeObject* smoke, const glm::mat4 & viewMatrix, const glm::mat4 & projectionMatrix);
void drawSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, bool day);
void drawRain(RainObject* rain, const glm::mat4 & viewMatrix, const glm::mat4 & projectionMatrix);

// -----------------------------------------------------------------------------------------------------------------------------------------------------

void cleanupShaderPrograms();
void clearGeometry(MeshGeometry* geometry);
void clearModels();

#endif // __RENDER_STUFF_H
