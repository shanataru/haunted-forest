//----------------------------------------------------------------------------------------
/**
*      file	|      render_stuff.cpp
*/
//----------------------------------------------------------------------------------------
#include <iostream>
#include "pgr.h"
#include "render_stuff.h"
#include "data.h"
#include "const.h"
#include "spline.h"

// mesh geometry for all object in scene
MeshGeometry* tree01MeshGeometry;
MeshGeometry* tree02MeshGeometry;
MeshGeometry* tree03MeshGeometry;
MeshGeometry* tree04MeshGeometry;
MeshGeometry* extraMeshGeometry;
MeshGeometry* extraNegMeshGeometry;
//MeshGeometry* skyboxDayMeshGeometry;
MeshGeometry* skyboxNightMeshGeometry;
MeshGeometry* groundMeshGeometry;
MeshGeometry* skullMeshGeometry;
MeshGeometry* mushroomMeshGeometry;
MeshGeometry* rainGeometry;
MeshGeometry* batMeshGeometry;
MeshGeometry* ghostMeshGeometry;
MeshGeometry* smokeGeometry;
MeshGeometry* rockMeshGeometry;

// used shader program
SCommonShaderProgram shaderProgram;
SSkyboxShaderProgram skyboxShaderProgram;
SRainShaderProgram rainShaderProgram;
SSmokeShaderProgram smokeShaderProgram;

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// LOAD MESH, SET UNIFORMS

/** Load mesh using assimp library
* \param filename [in] file to open/load
* \param shader [in] vao will connect loaded data to shader
* \param vbo [out] vertex and normal data |VVVVV...|NNNNN...| (no interleaving)
* \param eao [out] triangle indices
* \param vao [out] vao connects data to shader input
* \param numTriangles [out] how many triangles have been loaded and stored into index array eao
*/
bool loadSingleMesh(const std::string& fileName, SCommonShaderProgram& shader, MeshGeometry** geometry)
{
	Assimp::Importer importer;

	importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, 1); // Unitize object in size (scale the model to fit into (-1..1)^3)
																// Load asset from the file - you can play with various processing steps
	const aiScene* scn = importer.ReadFile(fileName.c_str(), 0
		| aiProcess_Triangulate // Triangulate polygons (if any).
		| aiProcess_PreTransformVertices // Transforms scene hierarchy into one root with geometry-leafs only. For more see Doc.
		| aiProcess_GenSmoothNormals // Calculate normals per vertex.
		| aiProcess_JoinIdenticalVertices);
	// abort if the loader fails
	if (scn == NULL) {
		std::cerr << "assimp error: " << importer.GetErrorString() << std::endl;
		*geometry = NULL;
		return false;
	}
	// some formats store whole scene (multiple meshes and materials, lights, cameras, ...) in one file, we cannot handle that in our simplified example
	if (scn->mNumMeshes != 1) {
		std::cerr << "this simplified loader can only process files with only one mesh" << std::endl;
		*geometry = NULL;
		return false;
	}
	// in this phase we know we have one mesh in our loaded scene, we can directly copy its data to opengl ...
	const aiMesh* mesh = scn->mMeshes[0];

	*geometry = new MeshGeometry;

	// vertex buffer object, store all vertex positions and normals
	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * mesh->mNumVertices, 0, GL_STATIC_DRAW); // allocate memory for vertices, normals, and texture coordinates
																								// first store all vertices
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * mesh->mNumVertices, mesh->mVertices);
	// then store all normals
	glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float) * mesh->mNumVertices, 3 * sizeof(float) * mesh->mNumVertices, mesh->mNormals);

	// just texture 0 for now
	float* textureCoords = new float[2 * mesh->mNumVertices]; // 2 floats per vertex
	float* currentTextureCoord = textureCoords;

	// copy texture coordinates
	aiVector3D vect;

	if (mesh->HasTextureCoords(0)) {
		// we use 2D textures with 2 coordinates and ignore the third coordinate
		for (unsigned int idx = 0; idx < mesh->mNumVertices; idx++) {
			vect = (mesh->mTextureCoords[0])[idx];
			*currentTextureCoord++ = vect.x;
			*currentTextureCoord++ = vect.y;
		}
	}

	// finally store all texture coordinates
	glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(float) * mesh->mNumVertices, 2 * sizeof(float) * mesh->mNumVertices, textureCoords);

	// copy all mesh faces into one big array (assimp supports faces with ordinary number of vertices, we use only 3 -> triangles)
	unsigned int* indices = new unsigned int[mesh->mNumFaces * 3];
	for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
		indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
		indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
		indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
	}
	// copy our temporary index array to OpenGL and free the array
	glGenBuffers(1, &((*geometry)->elementBufferObject));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned) * mesh->mNumFaces, indices, GL_STATIC_DRAW);

	delete[] indices;

	// copy the material info to MeshGeometry structure
	const aiMaterial* mat = scn->mMaterials[mesh->mMaterialIndex];
	aiColor3D color;
	aiString name;

	// Get returns: aiReturn_SUCCESS 0 | aiReturn_FAILURE -1 | aiReturn_OUTOFMEMORY -3
	mat->Get(AI_MATKEY_NAME, name); // may be "" after the input mesh processing. Must be aiString type!
	mat->Get<aiColor3D>(AI_MATKEY_COLOR_DIFFUSE, color);
	(*geometry)->diffuse = glm::vec3(color.r, color.g, color.b);
	mat->Get<aiColor3D>(AI_MATKEY_COLOR_AMBIENT, color);
	(*geometry)->ambient = glm::vec3(color.r, color.g, color.b);
	mat->Get<aiColor3D>(AI_MATKEY_COLOR_SPECULAR, color);
	(*geometry)->specular = glm::vec3(color.r, color.g, color.b);
	float shininess;

	mat->Get<float>(AI_MATKEY_SHININESS, shininess);
	(*geometry)->shininess = shininess / 4.0f; // shininess divisor-not descibed anywhere

	(*geometry)->texture = 0;

	// load texture image
	if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
		// get texture name
		mat->Get<aiString>(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), name);
		std::string textureName = name.data;

		size_t found = fileName.find_last_of("/\\");
		// insert correct texture file path
		if (found != std::string::npos) { // not found
											//subMesh_p->textureName.insert(0, "/");
			textureName.insert(0, fileName.substr(0, found + 1));
		}

		std::cout << "Loading texture file: " << textureName << std::endl;

		(*geometry)->texture = pgr::createTexture(textureName);
	}

	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject); // bind our element array buffer (indices) to vao
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);

	glEnableVertexAttribArray(shader.posLocation);
	glVertexAttribPointer(shader.posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(shader.texCoordLocation);

	glVertexAttribPointer(shader.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)(6 * sizeof(float) * mesh->mNumVertices));

	glEnableVertexAttribArray(shader.normalLocation);
	glVertexAttribPointer(shader.normalLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)(3 * sizeof(float) * mesh->mNumVertices));

	glBindVertexArray(0);

	(*geometry)->numTriangles = mesh->mNumFaces;

	return true;
}

/**
Sets uniforms for shaderProgram.
Uniforms set here: matrix for transformations and light uniforms
\param[in] modelMatrix
\param[in] viewMatrix
\param[in] projectionMatrix
*/
void setTransformUniforms(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{

	glm::mat4 PVM = projectionMatrix * viewMatrix * modelMatrix;
	glUniformMatrix4fv(shaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVM)); //value_ptr vraci pointer
	glUniformMatrix4fv(shaderProgram.VmatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(shaderProgram.MmatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(viewMatrix * modelMatrix));
	glUniformMatrix4fv(shaderProgram.normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix)); // correct matrix for non-rigid transform
}

/**
Sets uniforms for shaderProgram.
Uniforms set here: material and texture uniforms
\param[in] ambient
\param[in] specular
\param[in] shininess
\param[in] texture
*/
void setMaterialUniforms(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, GLuint texture)
{
	glUniform3fv(shaderProgram.diffuseLocation, 1, glm::value_ptr(diffuse)); // 2nd parameter must be 1 - it declares number of vectors in the vector array
	glUniform3fv(shaderProgram.ambientLocation, 1, glm::value_ptr(ambient));
	glUniform3fv(shaderProgram.specularLocation, 1, glm::value_ptr(specular));
	glUniform1f(shaderProgram.shininessLocation, shininess);

	if (texture != 0) {
		glUniform1i(shaderProgram.useTextureLocation, 1); // do texture sampling
		glUniform1i(shaderProgram.texSamplerLocation, 0); // texturing unit 0 -> samplerID   [for the GPU linker]
		glActiveTexture(GL_TEXTURE0 + 0); // texturing unit 0 -> to be bound [for OpenGL BindTexture]
		glBindTexture(GL_TEXTURE_2D, texture);
	}
	else {
		glUniform1i(shaderProgram.useTextureLocation, 0); // do not sample the texture
	}
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// INITIALIZATION

// inicialize shaders
void initializeShaderPrograms(void)
{
	std::vector<GLuint> shaderList;
	shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "vs.vert"));
	shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "fs.frag"));
	
	// create the program with two shaders
	shaderProgram.program = pgr::createProgram(shaderList);
	
	//world
	shaderProgram.posLocation = glGetAttribLocation(shaderProgram.program, "position");
	shaderProgram.normalLocation = glGetAttribLocation(shaderProgram.program, "normal");
	shaderProgram.texCoordLocation = glGetAttribLocation(shaderProgram.program, "texCoord");
	shaderProgram.colorLocation = glGetAttribLocation(shaderProgram.program, "color");
	shaderProgram.timeLocation = glGetUniformLocation(shaderProgram.program, "time");
	
	// matrix
	shaderProgram.VmatrixLocation = glGetUniformLocation(shaderProgram.program, "Vmatrix");
	shaderProgram.PVMmatrixLocation = glGetUniformLocation(shaderProgram.program, "PVMmatrix");
	shaderProgram.MmatrixLocation = glGetUniformLocation(shaderProgram.program, "Mmatrix");
	shaderProgram.normalMatrixLocation = glGetUniformLocation(shaderProgram.program, "normalMatrix");
	
	// material
	shaderProgram.ambientLocation = glGetUniformLocation(shaderProgram.program, "material.ambient");
	shaderProgram.diffuseLocation = glGetUniformLocation(shaderProgram.program, "material.diffuse");
	shaderProgram.specularLocation = glGetUniformLocation(shaderProgram.program, "material.specular");
	shaderProgram.shininessLocation = glGetUniformLocation(shaderProgram.program, "material.shininess");
	
	// texture
	shaderProgram.texSamplerLocation = glGetUniformLocation(shaderProgram.program, "texSampler");
	shaderProgram.useTextureLocation = glGetUniformLocation(shaderProgram.program, "material.useTexture");
	
	//reflector
	shaderProgram.reflectorOnLocation = glGetUniformLocation(shaderProgram.program, "reflectorOn");
	shaderProgram.reflectorPositionLocation = glGetUniformLocation(shaderProgram.program, "reflectorPosition");
	shaderProgram.reflectorDirectionLocation = glGetUniformLocation(shaderProgram.program, "reflectorDirection");
	
	//fog
	shaderProgram.fogOnLocation = glGetUniformLocation(shaderProgram.program, "fogOn");
	shaderProgram.fogColorLocation = glGetUniformLocation(shaderProgram.program, "fogColor");
	shaderProgram.fogDensityLocation = glGetUniformLocation(shaderProgram.program, "fogDensity");
	
	//sun
	shaderProgram.sunOnLocation = glGetUniformLocation(shaderProgram.program, "sunOn");
	
	//pointlight
	shaderProgram.pointlightPositionLocation = glGetUniformLocation(shaderProgram.program, "pointlightPosition");
	shaderProgram.pointlightOnLocation = glGetUniformLocation(shaderProgram.program, "pointlightOn");

	shaderList.clear();

	//rain
	shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "rain.vert"));
	shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "rain.frag"));
	rainShaderProgram.program = pgr::createProgram(shaderList);

	rainShaderProgram.posLocation = glGetAttribLocation(rainShaderProgram.program, "position");
	rainShaderProgram.texCoordLocation = glGetAttribLocation(rainShaderProgram.program, "texCoord");
	rainShaderProgram.timeLocation = glGetUniformLocation(rainShaderProgram.program, "time");
	rainShaderProgram.PVMmatrixLocation = glGetUniformLocation(rainShaderProgram.program, "PVMmatrix");
	rainShaderProgram.texSamplerLocation = glGetUniformLocation(rainShaderProgram.program, "texSampler");
	rainShaderProgram.texTransMatrixLocation = glGetUniformLocation(rainShaderProgram.program, "texTransMatrix");

	shaderList.clear();

	//skybox
	shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "skybox.vert"));
	shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "skybox.frag"));
	skyboxShaderProgram.program = pgr::createProgram(shaderList);

	skyboxShaderProgram.screenCoordLocation = glGetAttribLocation(skyboxShaderProgram.program, "screenCoord");

	skyboxShaderProgram.skyboxSamplerLocation = glGetUniformLocation(skyboxShaderProgram.program, "skyboxSampler");
	skyboxShaderProgram.inversePVmatrixLocation = glGetUniformLocation(skyboxShaderProgram.program, "inversePVmatrix");
	skyboxShaderProgram.fogOnLocation = glGetUniformLocation(skyboxShaderProgram.program, "fogOn");
	skyboxShaderProgram.fogColorLocation = glGetUniformLocation(shaderProgram.program, "fogColor");
	skyboxShaderProgram.fogDensityLocation = glGetUniformLocation(shaderProgram.program, "fogDensity");

	shaderList.clear();

	//smoke
	shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "smoke.vert"));
	shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "smoke.frag"));
	smokeShaderProgram.program = pgr::createProgram(shaderList);

	smokeShaderProgram.posLocation = glGetAttribLocation(smokeShaderProgram.program, "position");
	smokeShaderProgram.texCoordLocation = glGetAttribLocation(smokeShaderProgram.program, "texCoord");

	smokeShaderProgram.timeLocation = glGetUniformLocation(smokeShaderProgram.program, "time");
	smokeShaderProgram.PVMmatrixLocation = glGetUniformLocation(smokeShaderProgram.program, "PVMmatrix");
	smokeShaderProgram.VmatrixLocation = glGetUniformLocation(smokeShaderProgram.program, "Vmatrix");
	smokeShaderProgram.texSamplerLocation = glGetUniformLocation(smokeShaderProgram.program, "texSampler");
	smokeShaderProgram.frameDurationLocation = glGetUniformLocation(smokeShaderProgram.program, "frameDuration");
	
	shaderList.clear();
}

// init ground - material
void initgroundMeshGeometry(SCommonShaderProgram& shader, MeshGeometry** geometry)
{
	*geometry = new MeshGeometry;
	(*geometry)->texture = pgr::createTexture(GROUND_TEXTURE);
	//CHECK_GL_ERROR();
	(*geometry)->ambient = glm::vec3(0.520f, 0.34f, 0.38f);
	(*geometry)->diffuse = glm::vec3(1.0f, 1.0f, 0.7f);
	(*geometry)->specular = glm::vec3(1.0f, 1.0f, 1.0f);
	(*geometry)->shininess = 0.7f;
	(*geometry)->numTriangles = planeNTriangles;

	//VAO
	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);

	//VBO
	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

	//EBO
	glGenBuffers(1, &((*geometry)->elementBufferObject));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeTriangles), planeTriangles, GL_STATIC_DRAW);

	// enable and initialize the attributes array 
	glEnableVertexAttribArray(shader.posLocation);
	glVertexAttribPointer(shader.posLocation, 3, GL_FLOAT, GL_FALSE, planeNAttribsPerVertex * sizeof(float), 0);

	glEnableVertexAttribArray(shader.normalLocation);
	glVertexAttribPointer(shader.normalLocation, 3, GL_FLOAT, GL_FALSE, planeNAttribsPerVertex * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(shader.texCoordLocation);
	glVertexAttribPointer(shader.texCoordLocation, 2, GL_FLOAT, GL_FALSE, planeNAttribsPerVertex * sizeof(float), (void*)(6 * sizeof(float)));

	glBindVertexArray(0);
}

// init rain
void initRainGeometry(GLuint shader, MeshGeometry **geometry) 
{
	*geometry = new MeshGeometry;
	(*geometry)->texture = pgr::createTexture(RAIN_TEXTURE);
	(*geometry)->numTriangles = 2;

	//VAO
	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));		
	glBindVertexArray((*geometry)->vertexArrayObject);
	glGenBuffers(1, &((*geometry)->vertexBufferObject));	
	//VBO
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rain), rain, GL_STATIC_DRAW);

	glEnableVertexAttribArray(rainShaderProgram.posLocation);
	//vertices of triangles - start at the beginning of the array
	glVertexAttribPointer(rainShaderProgram.posLocation, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 0);

	glEnableVertexAttribArray(rainShaderProgram.texCoordLocation);
	//texture coords
	glVertexAttribPointer(rainShaderProgram.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));

	glBindTexture(GL_TEXTURE_2D, (*geometry)->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glBindVertexArray(0);
}

//init smoke
void initSmokeGeometry(GLuint shader, MeshGeometry**geometry)
{
	*geometry = new MeshGeometry;

	(*geometry)->texture = pgr::createTexture(SMOKE_TEXTURE);
	(*geometry)->numTriangles = smokeNumQuadVertices;

	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);
	glGenBuffers(1, &((*geometry)->vertexBufferObject));

	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(smoke), smoke, GL_STATIC_DRAW);

	glEnableVertexAttribArray(smokeShaderProgram.posLocation);
	glVertexAttribPointer(smokeShaderProgram.posLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

	glEnableVertexAttribArray(smokeShaderProgram.texCoordLocation);
	glVertexAttribPointer(smokeShaderProgram.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}

//init rock - material
void initrockMeshGeometry(SCommonShaderProgram& shader, MeshGeometry** geometry)
{
	*geometry = new MeshGeometry;
	(*geometry)->texture = pgr::createTexture(ROCK_TEXTURE);
	(*geometry)->ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	(*geometry)->diffuse = glm::vec3(0.86f, 0.85f, 0.84f);
	(*geometry)->specular = glm::vec3(0.18f, 0.31f, 0.31f);
	(*geometry)->shininess = 0.7f;
	(*geometry)->numTriangles = rockNTriangles;

	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);

	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rockVertices), rockVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &((*geometry)->elementBufferObject));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rockTriangles), rockTriangles, GL_STATIC_DRAW);

	// enable and initialize the attributes array 
	glEnableVertexAttribArray(shader.posLocation);
	glVertexAttribPointer(shader.posLocation, 3, GL_FLOAT, GL_FALSE, rockNAttribsPerVertex * sizeof(float), 0);

	glEnableVertexAttribArray(shader.normalLocation);
	glVertexAttribPointer(shader.normalLocation, 3, GL_FLOAT, GL_FALSE, rockNAttribsPerVertex * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(shader.texCoordLocation);
	glVertexAttribPointer(shader.texCoordLocation, 2, GL_FLOAT, GL_FALSE, rockNAttribsPerVertex * sizeof(float), (void*)(6 * sizeof(float)));

	glBindVertexArray(0);
}

// init skybox
void initskyboxMeshGeometry(GLuint shader, MeshGeometry** geometry, bool day)
{
	*geometry = new MeshGeometry;

	// 2D coordinates of 2 triangles covering the whole screen (NDC), draw using triangle strip
	static const float screenCoords[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f,  1.0f,
		1.0f,  1.0f
	};

	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);

	// buffer for far plane rendering
	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenCoords), screenCoords, GL_STATIC_DRAW);

	glEnableVertexAttribArray(skyboxShaderProgram.screenCoordLocation);
	glVertexAttribPointer(skyboxShaderProgram.screenCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
	glUseProgram(0);
	CHECK_GL_ERROR();

	(*geometry)->numTriangles = 2;

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &((*geometry)->texture));
	glBindTexture(GL_TEXTURE_CUBE_MAP, (*geometry)->texture);

	const char * suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
	GLuint targets[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	std::string texName;
	for (int i = 0; i < 6; i++) 
	{
		if (day)
			texName = std::string(SKYBOX_CUBE_TEXTURE_FILE_PREFIX_DAY) + "_" + suffixes[i] + ".jpg";
		else
			texName = std::string(SKYBOX_CUBE_TEXTURE_FILE_PREFIX_NIGHT) + "_" + suffixes[i] + ".jpg";

		std::cout << "Loading cube map texture: " << texName << std::endl;
		if (!pgr::loadTexImage2D(texName, targets[i])) {
			pgr::dieWithError("Skybox cube map loading failed!");
		}
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// unbind the texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	CHECK_GL_ERROR();
}

// initialize all models used in scene
void initializeModels()
{
	initgroundMeshGeometry(shaderProgram, &groundMeshGeometry);
	initRainGeometry(rainShaderProgram.program, &rainGeometry);
	initSmokeGeometry(smokeShaderProgram.program, &smokeGeometry);
	initrockMeshGeometry(shaderProgram, &rockMeshGeometry);
	//initskyboxMeshGeometry(skyboxShaderProgram.program, &skyboxDayMeshGeometry, true);
	initskyboxMeshGeometry(skyboxShaderProgram.program, &skyboxNightMeshGeometry, false);

	// load models from external file
	if (loadSingleMesh(TREE_MODEL_01, shaderProgram, &tree01MeshGeometry) != true)
		std::cerr << "Tree model 01 loading failed" << std::endl;
	if (loadSingleMesh(TREE_MODEL_02, shaderProgram, &tree02MeshGeometry) != true)
		std::cerr << "Tree model 02 loading failed" << std::endl;
	if (loadSingleMesh(TREE_MODEL_03, shaderProgram, &tree03MeshGeometry) != true)
		std::cerr << "Tree model 03 loading failed" << std::endl;
	if (loadSingleMesh(TREE_MODEL_04, shaderProgram, &tree04MeshGeometry) != true)
		std::cerr << "Tree model 04 loading failed" << std::endl;
	if (loadSingleMesh(SKULL_MODEL, shaderProgram, &skullMeshGeometry) != true)
		std::cerr << "Skull model loading failed" << std::endl;
	if (loadSingleMesh(MUSHROOM_MODEL, shaderProgram, &mushroomMeshGeometry) != true)
		std::cerr << "Mushroom model loading failed" << std::endl;
	if (loadSingleMesh(BAT_MODEL, shaderProgram, &batMeshGeometry) != true)
		std::cerr << "Bat model loading failed" << std::endl;
	if (loadSingleMesh(GHOST_MODEL, shaderProgram, &ghostMeshGeometry) != true)
		std::cerr << "Ghost model loading failed" << std::endl;

	if (loadSingleMesh(EXTRA_OBJECT_MODEL, shaderProgram, &extraMeshGeometry) != true)
		std::cerr << "Extra object model loading failed" << std::endl;
	if (loadSingleMesh(EXTRANEG_OBJECT_MODEL, shaderProgram, &extraNegMeshGeometry) != true)
		std::cerr << "Extraneg object model loading failed" << std::endl;

	//change ghosts materials
	ghostMeshGeometry->ambient = glm::vec3(1.0f, 1.0f, 1.0f);
	ghostMeshGeometry->diffuse = glm::vec3(1.0f, 0.0f, 1.0f);
	ghostMeshGeometry->specular = glm::vec3(1.0f, 0.0f, 1.0f);
	ghostMeshGeometry->shininess = 20.0f;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// DRAW 

// draw ground
void drawGround(GroundObject* ground, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	glUseProgram(shaderProgram.program);

	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), ground->position);
	modelMatrix = glm::rotate(modelMatrix, ground->viewAngle, glm::vec3(0, 0, 1));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(ground->size.x, ground->size.y, ground->size.z));
	
	// setting matrices to the vertex & fragment shader
	setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);
	setMaterialUniforms(groundMeshGeometry->ambient, groundMeshGeometry->diffuse, groundMeshGeometry->specular, groundMeshGeometry->shininess, groundMeshGeometry->texture);
	
	glBindVertexArray(groundMeshGeometry->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, groundMeshGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}

// draw rock
void drawRock(Object* rock, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	glUseProgram(shaderProgram.program);

	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), rock->position);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.02f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(rock->size));

	// setting matrices to the vertex & fragment shader
	setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);
	setMaterialUniforms(rockMeshGeometry->ambient, rockMeshGeometry->diffuse, rockMeshGeometry->specular, rockMeshGeometry->shininess, rockMeshGeometry->texture);

	glBindVertexArray(rockMeshGeometry->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, rockMeshGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}

// draw bat
void drawBat(MovingObject* bat, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	glUseProgram(shaderProgram.program);
	
	glm::mat4 modelMatrix = alignObject(bat->position, bat->direction, glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::rotate(modelMatrix, 180.0f, glm::vec3(0, 1, 0)); //otoceny model
	modelMatrix = glm::scale(modelMatrix, glm::vec3(bat->size, bat->size, bat->size));
	
	setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);
	setMaterialUniforms(batMeshGeometry->ambient, batMeshGeometry->diffuse, batMeshGeometry->specular, batMeshGeometry->shininess, batMeshGeometry->texture);
	
	glBindVertexArray(batMeshGeometry->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, batMeshGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);
	
	glBindVertexArray(0);
	glUseProgram(0);	
}

// draw ghost
void drawGhost(MovingObject * ghost, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	glUseProgram(shaderProgram.program);

	glm::mat4 modelMatrix = alignObject(ghost->position, ghost->direction, glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::rotate(modelMatrix, 180.0f, glm::vec3(0, 1, 0)); //otoceny model
	modelMatrix = glm::scale(modelMatrix, glm::vec3(ghost->size, ghost->size, ghost->size));

	setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);
	setMaterialUniforms(ghostMeshGeometry->ambient, ghostMeshGeometry->diffuse, ghostMeshGeometry->specular, ghostMeshGeometry->shininess, ghostMeshGeometry->texture);

	glBindVertexArray(ghostMeshGeometry->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, ghostMeshGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}

// draw rain
void drawRain(RainObject* rain, const glm::mat4 & viewMatrix, const glm::mat4 & projectionMatrix) 
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(rainShaderProgram.program);
	
	glm::mat4 modelMatrix = alignObject(rain->position, rain->direction, glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f*rain->size, 2.0f*rain->size, rain->size));
	
	glm::mat4 PVMmatrix = projectionMatrix * viewMatrix * modelMatrix;
	glUniformMatrix4fv(rainShaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVMmatrix)); // model-view-projection
	glUniform1f(rainShaderProgram.timeLocation, rain->currentTime);
	glUniform1i(rainShaderProgram.texSamplerLocation, 0); //info for GLSL - which texture unit

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rainGeometry->texture);

	glm::mat4 TTmatrix = glm::mat4  //texture transform matrix 
	(1.0f, 0.0f, 0.0f, 0.0f, 
	 0.0f, 1.0f, 0.0f, 0.0f,
	 0.0f, 0.0f, 1.0f, 0.0f,
	 0.0f, 0.0f, 0.0f, 1.0f);

	int frame = (rain->currentTime * 75);
	float delta = 0.005f*(frame % 1000);
	// update y ~ rains from up to down
	TTmatrix[2].y = delta;

	glUniformMatrix4fv(rainShaderProgram.texTransMatrixLocation, 1, GL_FALSE, glm::value_ptr(TTmatrix));
	glBindVertexArray(rainGeometry->vertexArrayObject);
	glDrawArrays(GL_TRIANGLES, 0, 3 * rainGeometry->numTriangles);

	//CHECK_GL_ERROR();
	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);
}

// draw MeshGeometry - used for trees, skull, mushroom - still objects
void drawMeshGeometry(MeshGeometry* geometry, glm::vec3 position, glm::vec3 direction, float size, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	glUseProgram(shaderProgram.program);

	glm::mat4 modelMatrix = alignObject(position, direction, glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.2f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(size));

	setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);
	setMaterialUniforms(geometry->ambient, geometry->diffuse, geometry->specular, geometry->shininess, geometry->texture);

	glBindVertexArray(geometry->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, geometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}

// draw tree
void drawTree(Object * tree, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, int type)
{
	switch (type)
	{
	case 1:
		drawMeshGeometry(tree01MeshGeometry, tree->position, tree->direction, tree->size, viewMatrix, projectionMatrix);
		break;
	case 2:
		drawMeshGeometry(tree02MeshGeometry, tree->position, tree->direction, tree->size, viewMatrix, projectionMatrix);
		break;
	case 3:
		drawMeshGeometry(tree03MeshGeometry, tree->position, tree->direction, tree->size, viewMatrix, projectionMatrix);
		break;
	case 4:
		drawMeshGeometry(tree04MeshGeometry, tree->position, tree->direction, tree->size, viewMatrix, projectionMatrix);
		break;
	default:
		break;
	}
}

//draw extra objects
void drawExtra(Object* extra, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, bool diffColor)
{
	if (!diffColor)
		drawMeshGeometry(extraMeshGeometry, extra->position, extra->direction, extra->size, viewMatrix, projectionMatrix);
	else
		drawMeshGeometry(extraNegMeshGeometry, extra->position, extra->direction, extra->size, viewMatrix, projectionMatrix);
}

// draw skull
void drawSkull(Object* skull, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	drawMeshGeometry(skullMeshGeometry, skull->position, skull->direction, skull->size, viewMatrix, projectionMatrix);
}

// draw mushroom
void drawMushroom(Object* mush, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	drawMeshGeometry(mushroomMeshGeometry, mush->position, mush->direction, mush->size, viewMatrix, projectionMatrix);
}

//draw smoke
void drawSmoke(SmokeObject * smoke, const glm::mat4 & viewMatrix, const glm::mat4 & projectionMatrix)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glUseProgram(smokeShaderProgram.program);

	// just take rotation part of the view transform
	glm::mat4 billboardRotationMatrix = glm::mat4(
		viewMatrix[0],
		viewMatrix[1],
		viewMatrix[2],
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) //W = 1!!!!!!
	);
	// inverse view rotation
	billboardRotationMatrix = glm::transpose(billboardRotationMatrix);

	glm::mat4 matrix = glm::translate(glm::mat4(1.0f), smoke->position);
	matrix = glm::scale(matrix, glm::vec3(smoke->size));
	matrix = matrix*billboardRotationMatrix; // make billboard to face the camera

	glm::mat4 PVMmatrix = projectionMatrix * viewMatrix * matrix;
	glUniformMatrix4fv(smokeShaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVMmatrix));  // model-view-projection
	glUniformMatrix4fv(smokeShaderProgram.VmatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));   // view
	glUniform1f(smokeShaderProgram.timeLocation, smoke->currentTime - smoke->startTime);
	glUniform1i(smokeShaderProgram.texSamplerLocation, 0);
	glUniform1f(smokeShaderProgram.frameDurationLocation, smoke->frameDuration);

	glBindVertexArray(smokeGeometry->vertexArrayObject);
	glBindTexture(GL_TEXTURE_2D, smokeGeometry->texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, smokeGeometry->numTriangles);

	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
}

// draw skybox
void drawSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, bool sunOn)
{
	glUseProgram(skyboxShaderProgram.program);

	// compose transformations
	glm::mat4 matrix = projectionMatrix * viewMatrix;

	// crate view rotation matrix by using view matrix with cleared translation
	glm::mat4 viewRotation = viewMatrix;
	viewRotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// vertex shader will translate screen space coordinates (NDC) using inverse PV matrix
	glm::mat4 inversePVmatrix = glm::inverse(projectionMatrix * viewRotation);

	glUniformMatrix4fv(skyboxShaderProgram.inversePVmatrixLocation, 1, GL_FALSE, glm::value_ptr(inversePVmatrix));
	glUniform1i(skyboxShaderProgram.skyboxSamplerLocation, 0);

	// draw "skybox" rendering 2 triangles covering the far plane
	glBindVertexArray(skyboxNightMeshGeometry->vertexArrayObject);
	//glBindVertexArray(skyboxDayMeshGeometry->vertexArrayObject);
	
	//one skybox (night)
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxNightMeshGeometry->texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, skyboxNightMeshGeometry->numTriangles + 2);

	//two skyboxes (day/night)
	/*if (sunOn) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxDayMeshGeometry->texture);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, skyboxDayMeshGeometry->numTriangles + 2);
	}
	else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxNightMeshGeometry->texture);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, skyboxNightMeshGeometry->numTriangles + 2);
	}*/

	glBindVertexArray(0);
	glUseProgram(0);
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// CLEAN UP

// clean shaders
void cleanupShaderPrograms(void)
{
	pgr::deleteProgramAndShaders(shaderProgram.program);
	pgr::deleteProgramAndShaders(skyboxShaderProgram.program);
	pgr::deleteProgramAndShaders(rainShaderProgram.program);
	pgr::deleteProgramAndShaders(smokeShaderProgram.program);
}

// clear geometry = clear buffers of geometry
void clearGeometry(MeshGeometry* geometry)
{
	glDeleteVertexArrays(1, &(geometry->vertexArrayObject));
	glDeleteBuffers(1, &(geometry->elementBufferObject));
	glDeleteBuffers(1, &(geometry->vertexBufferObject));
}

// clear all models used in scene
void clearModels()
{
	clearGeometry(tree01MeshGeometry);
	clearGeometry(tree02MeshGeometry);
	clearGeometry(tree03MeshGeometry);
	clearGeometry(tree04MeshGeometry);
	clearGeometry(extraMeshGeometry);
	clearGeometry(extraNegMeshGeometry);
	clearGeometry(groundMeshGeometry);
	//clearGeometry(skyboxDayMeshGeometry);
	clearGeometry(skyboxNightMeshGeometry);
	clearGeometry(skullMeshGeometry);
	clearGeometry(mushroomMeshGeometry);
	clearGeometry(rainGeometry);
	clearGeometry(batMeshGeometry);
	clearGeometry(ghostMeshGeometry);
	clearGeometry(smokeGeometry);
	clearGeometry(rockMeshGeometry);
}