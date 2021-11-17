//----------------------------------------------------------------------------------------
/**
*		file	|       main.cpp
*		author	|		Ngyenová Giang Chau
* 		date	|		2. 4. 2017
*/
//----------------------------------------------------------------------------------------

#include <time.h>
#include <list>
#include <vector>
#include <iostream>
#include <stdlib.h> 
#include "pgr.h"
#include "const.h"
#include "render_stuff.h"
#include "spline.h"

//set shader uniforms here
extern SCommonShaderProgram shaderProgram;
extern SSkyboxShaderProgram skyboxShaderProgram;

typedef std::list<void*> GameObjectsList;

std::vector<glm::vec3> GameObjectsPositions;

//structure for state of app
struct GameState 
{
	// application window width x height
	int windowWidth;
	int windowHeight;

	bool cameraSetup;				// if camera is set to static
	int cameraNumber;				// number of camera view (3)
	bool freeCameraMode;			// free camera mode
	float cameraElevationAngle;		// elevation angle of camera
	bool sunOn;						// if is sun on --- moonlight
	bool sunForced;					//sun turned on by user
	bool rain;						// if its raining
	bool reflectorOn;				// if is reflector on
	int attemptCnt;					//number of times catching the mushroom
	bool diffColor;					//extra object in different color
	bool ghost;						//if ghost is spawned
	bool keyMap[KEYS_COUNT];		// map of specail keys
	float elapsedTime;				// app elapsed time
} gameState;

//Structure of all game objects
struct GameObjects 
{
	CameraObject* camera;
	GroundObject * ground;

	//4 types of tree
	GameObjectsList trees01;
	GameObjectsList trees02;
	GameObjectsList trees03;
	GameObjectsList trees04;

	//extra object - mushrooms
	GameObjectsList extra;

	Object * skull;
	Object * mush;
	Object * rock;
	
	RainObject * rain;
	FogObject * fog;
	SmokeObject* smoke;

	//3 flying bats + 1 ghost
	MovingObject * bat01;
	MovingObject * bat02;
	MovingObject * bat03;
	MovingObject * ghost;

} gameObjects;


// -----------------------------------------------------------------------------------------------------------------------------------------------------
// turn camera left 
void turnCameraLeft(float deltaAngle)
{
	gameObjects.camera->viewAngle += deltaAngle;
	if (gameObjects.camera->viewAngle > 360.0f)
		gameObjects.camera->viewAngle -= 360.0f;

	float angle = glm::radians(gameObjects.camera->viewAngle);
	gameObjects.camera->direction = glm::vec3(cos(angle), sin(angle), 0.0f);
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// turn camera right
void turnCameraRight(float deltaAngle)
{
	gameObjects.camera->viewAngle -= deltaAngle;
	if (gameObjects.camera->viewAngle < 0.0f)
		gameObjects.camera->viewAngle += 360.0f;
	float angle = glm::radians(gameObjects.camera->viewAngle);
	gameObjects.camera->direction = glm::vec3(cos(angle), sin(angle), 0.0f);
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// clean objects
void cleanUpObjects(void)
{
	// delete game objects in list
	while (!gameObjects.trees01.empty())
	{
		delete gameObjects.trees01.back();
		gameObjects.trees01.pop_back();
	}

	while (!gameObjects.trees02.empty())
	{
		delete gameObjects.trees02.back();
		gameObjects.trees02.pop_back();
	}

	while (!gameObjects.trees03.empty())
	{
		delete gameObjects.trees03.back();
		gameObjects.trees03.pop_back();
	}

	while (!gameObjects.trees04.empty())
	{
		delete gameObjects.trees04.back();
		gameObjects.trees04.pop_back();
	}

	while (!gameObjects.extra.empty())
	{
		delete gameObjects.extra.back();
		gameObjects.extra.pop_back();
	}

	gameObjects.skull = NULL;
	gameObjects.mush = NULL;
	gameObjects.rock = NULL;
	gameObjects.rain = NULL;
	gameObjects.fog = NULL;
	gameObjects.bat01 = NULL;
	gameObjects.bat02 = NULL;
	gameObjects.bat03 = NULL;
	gameObjects.ghost = NULL;
	gameObjects.smoke = NULL;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// check for collisions
bool isCollision(const glm::vec3 positionToCheck)
{
	glm::vec2 a = glm::vec2(positionToCheck.x, positionToCheck.y);

	//check for starting camera collision
	if (gameState.cameraNumber != 0)
	{
		glm::vec2 centerCamera = glm::vec2(0.0f, 0.0f);
		float centerCamDist = sqrt(pow((float)(a.x - centerCamera.x), 2) + pow((float)(a.y - centerCamera.y), 2));
		if (centerCamDist <= TRESHOLD_RADIUS)
			return true;
	}

	//check for collision with the rest of the objects in the scene
	for (std::vector<glm::vec3>::iterator it = GameObjectsPositions.begin(); it != GameObjectsPositions.end(); ++it) 
	{
		glm::vec2 b = glm::vec2((*it).x, (*it).y);
		float distance = sqrt(pow((float)(a.x - b.x), 2) + pow((float)(a.y - b.y), 2));
		if (distance <= TRESHOLD_RADIUS)
			return true;
	}
	return false;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// generate random position
glm::vec3 generateRandomPosition(int type, float tree_size, bool savePosition)
{
	// position is generated randomly with z being 0.2f ("on ground")
	// coordinates are in range -4.0f ... 4.0f (x, y)

	glm::vec3 newPosition;
	do {
		newPosition = glm::vec3((float)((rand() / (double)(RAND_MAX + 1)) - 4.0f) + (double)(rand() % 8),
			(float)((rand() / (double)(RAND_MAX + 1)) - 4.0) + (double)(rand() % 8), 0.2f);
	} while (isCollision(newPosition) == true);

	switch (type) //depends on what type of tree and its size, change its z coord
	{
	case 2:
		newPosition.z = (float)(-0.28f + (tree_size * 10.0 * 0.08)); //ofs: -0.1, diff: 0.08
		break;
	default:
		newPosition.z = (float)(tree_size - 0.29); //ofs: -0.08, diff: 0.1
		break;
	}
	
	if (savePosition)
		GameObjectsPositions.push_back(newPosition); //save this "unique" position
	return newPosition;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// generate random direction
glm::vec3 generateRandomDirection(void)
{
	glm::vec3 newDirection;
	newDirection = glm::vec3((float)(2.0 * (rand() / (double)RAND_MAX) - 1.0), (float)(2.0 * (rand() / (double)RAND_MAX) - 1.0), 0.0f);
	return newDirection;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// generate random size of tree (from 0.2 to 0.5)
float generaterandomSize(void)
{
	float newsize = ((rand() % 4) + 2) / 10.0f;
	return newsize;
}

//----------------------------------------------------------------------------------------
// create tree objects ~ generates a tree object with random size (0.2f - 0.5f) and position
Object * createTree(int type)
{
	Object * newTree = new Object;
	newTree->size = generaterandomSize();
	newTree->direction = generateRandomDirection();
	newTree->direction = glm::normalize(newTree->direction);
	newTree->position = generateRandomPosition(type, newTree->size, true);
	return newTree;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// create skull object
Object * createSkull(void)
{
	Object * newSkull = new Object;
	newSkull->size = SKULL_SIZE;
	newSkull->direction = generateRandomDirection();
	newSkull->direction = glm::normalize(newSkull->direction);
	do 	//generate in reachable area
	{
		newSkull->position = generateRandomPosition(1, 0, true);
	} while (fabs(newSkull->position.x) >= (SCENE_WIDTH - 1.0f) || fabs(newSkull->position.y) >= (SCENE_HEIGHT - 1.0f));
	newSkull->position.z = -0.25f;

	return newSkull;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// create mushroom object AS LAST OBJECT!
Object * createMushroom(void)
{
	Object* newMush = new Object;
	newMush->size = MUSH_SIZE;
	newMush->direction = generateRandomDirection();
	newMush->direction = glm::normalize(newMush->direction);
	do 	//generate in reachable area
	{
		newMush->position = generateRandomPosition(1, 0, false); //false - do not save its position
	} while (fabs(newMush->position.x) >= SCENE_WIDTH || fabs(newMush->position.y) >= SCENE_HEIGHT);
	newMush->position.z = -0.23f;

	return newMush;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// create extra objects
Object * createExtra(void)
{
	Object * newEx = new Object;
	newEx->size = EXTRA_OBJECT_SIZE;
	newEx->direction = generateRandomDirection();
	newEx->direction = glm::normalize(newEx->direction);
	do 	//generate in reachable area
	{
		newEx->position = generateRandomPosition(1, 0, true); //false - do not save its position
	} while (fabs(newEx->position.x) >= (SCENE_WIDTH - 1.0f) || fabs(newEx->position.y) >= (SCENE_HEIGHT - 1.0f));
	newEx->position.z = -0.23f;
	return newEx;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// create ground object
GroundObject * createGround(void)
{
	GroundObject* newGround = new GroundObject;
	newGround->position = glm::vec3(0.0f, 0.0f, -0.08f);
	newGround->viewAngle = 0.0f;
	newGround->direction = glm::vec3(cos(glm::radians(newGround->viewAngle)), sin(glm::radians(newGround->viewAngle)), 0.0f);
	newGround->size = glm::vec3(5.0f, 5.0f, 0.05f);
	return newGround;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// create rock object
Object * createRock(void)
{
	Object * newRock = new Object;
	newRock->size = ROCK_SIZE;
	newRock->direction = generateRandomDirection();
	do 	//generate in reachable area
	{
		newRock->position = generateRandomPosition(1, 0, true); //false - do not save its position
	} while (fabs(newRock->position.x) >= (SCENE_WIDTH - 1.0f) || fabs(newRock->position.y) >= (SCENE_HEIGHT - 1.0f));
	newRock->position.z = -0.05f;
	return newRock;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// create rain object
RainObject* createRain(void)
{
	RainObject* newRain = new RainObject;
	newRain->size = RAIN_SIZE;
	newRain->position = gameObjects.camera->position + gameObjects.camera->direction*0.1f;
	newRain->direction = glm::normalize(gameObjects.camera->position - newRain->position);
	newRain->startTime = gameState.elapsedTime;
	newRain->currentTime = newRain->startTime;
	return newRain;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// create fog object
FogObject* createFog(void)
{
	FogObject* newFog = new FogObject;
	newFog->color = glm::vec4(0.53f, 0.13f, 0.13f, 1.0f);
	newFog->density = FOG_DENSITY;
	newFog->fogOn = false;
	return newFog;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Create bat object
MovingObject * createBat(int type)
{
	MovingObject* newBat = new MovingObject;
	newBat->position = glm::vec3(0.0f, 0.0f, 0.0f);
	newBat->initPosition = glm::vec3(0.0f, 1.0f, 0.0f);
	newBat->viewAngle = 0.0f;
	newBat->direction = glm::vec3(cos(glm::radians(newBat->viewAngle)), sin(glm::radians(newBat->viewAngle)), 0.0f);
	//newBat->directionForReflector = glm::vec3(sin(glm::radians(newBat->viewAngle)), 0.0f, -cos(glm::radians(newBat->viewAngle)));

	//depending on type, assign its speed and size
	if (type == 1)
	{
		newBat->speed = BAT_SPEED1;
		newBat->size = BAT_SIZE1;
	}
	else if (type == 2)
	{
		newBat->speed = BAT_SPEED2;
		newBat->size = BAT_SIZE2;
	}
	else
	{
		newBat->speed = BAT_SPEED3;
		newBat->size = BAT_SIZE3;
	}

	newBat->startTime = gameState.elapsedTime;
	newBat->currentTime = newBat->startTime;
	return newBat;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Create ghost object
MovingObject * createGhost(void)
{
	MovingObject * newGhost = new MovingObject;
	newGhost->position = glm::vec3(0.0f, 0.0f, 0.0f);
	//newGhost->position.z = 0.0f;
	newGhost->initPosition = glm::vec3(0.0f, 1.0f, 0.0f);
	//newGhost->initPosition.z = 0.0f;
	newGhost->viewAngle = 0.0f;
	newGhost->direction = glm::vec3(cos(glm::radians(newGhost->viewAngle)), sin(glm::radians(newGhost->viewAngle)), 0.0f);
	newGhost->size = GHOST_SIZE;
	newGhost->speed = GHOST_SPEED;
	newGhost->startTime = gameState.elapsedTime;
	newGhost->currentTime = newGhost->startTime;
	return newGhost;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Create smoke after clicking on skull
SmokeObject * createSmoke(const glm::vec3 & position)
{
	SmokeObject * newSmoke = new SmokeObject;
	newSmoke->position = position;
	newSmoke->direction = glm::vec3(0.0f, 0.0f, 1.0f);
	newSmoke->size = SMOKE_SIZE;
	newSmoke->speed = 0.0f;
	newSmoke->startTime = gameState.elapsedTime;
	newSmoke->currentTime = newSmoke->startTime;
	newSmoke->frameDuration = 0.09f;
	newSmoke->texFrames = 16;
	return newSmoke;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// setup of camera - 3 statics
void setupCamera(void)
{
	gameObjects.camera->viewAngle = 45.0f;
	float angle = glm::radians(gameObjects.camera->viewAngle);
	gameObjects.camera->direction = glm::vec3(cos(angle), sin(angle), 0.0f);
	gameObjects.camera->startTime = gameState.elapsedTime;
	gameObjects.camera->currentTime = gameObjects.camera->startTime;

	switch (gameState.cameraNumber) {
	case 0:
		gameObjects.camera->position = glm::vec3(0.0f, 0.0f, 0.09f);
		gameState.cameraElevationAngle = 10.0f;
		break;
	case 1:
		gameObjects.camera->position = glm::vec3(0.0f, 0.0f, 1.0f);
		gameState.cameraElevationAngle = 30.0f;
		break;
	case 2:
		gameObjects.camera->position = glm::vec3(-3.0f, -3.0f, 0.8f);
		gameState.cameraElevationAngle = 4.0f;
	default:
		break;
	}
	gameState.cameraSetup = false;
}

// restart
void restart(void)
{
	cleanUpObjects();
	gameState.elapsedTime = 0.001f * (float)glutGet(GLUT_ELAPSED_TIME); // milliseconds => seconds

	//setup a new camera
	gameState.cameraNumber = 0;
	if (gameObjects.camera == NULL)
		gameObjects.camera = new CameraObject;
	setupCamera();

	gameState.sunOn = false;
	gameState.sunForced = false;
	gameState.reflectorOn = false;
	gameState.rain = false;
	gameState.attemptCnt = 0;
	gameState.ghost = false;
	gameState.diffColor = false;

	if (gameState.freeCameraMode == true) 
	{
		gameState.freeCameraMode = false;
		glutPassiveMotionFunc(NULL);
	}

	//create objects in scene
	if (gameObjects.rain == NULL)
		gameObjects.rain = createRain();

	if (gameObjects.fog == NULL)
		gameObjects.fog = createFog();

	if (gameObjects.skull == NULL)
		gameObjects.skull = createSkull();

	if (gameObjects.mush == NULL)
		gameObjects.mush = createMushroom();

	if (gameObjects.ground == NULL)
		gameObjects.ground = createGround();

	if (gameObjects.rock == NULL)
		gameObjects.rock = createRock();

	// create trees01-04 and put them in list
	for (int i = 0; i < TREES01_COUNT; i++) {
		Object * newTree = createTree(1);
		gameObjects.trees01.push_back(newTree);
	}

	for (int i = 0; i < TREES02_COUNT; i++) {
		Object * newTree = createTree(2);
		gameObjects.trees02.push_back(newTree);
	}

	for (int i = 0; i < TREES03_COUNT; i++) {
		Object * newTree = createTree(3);
		gameObjects.trees03.push_back(newTree);
	}

	for (int i = 0; i < TREES04_COUNT; i++) {
		Object * newTree = createTree(4);
		gameObjects.trees04.push_back(newTree);
	}

	for (int i = 0; i < EXTRA_OBJECT_COUNT; i++) {
		Object * newEx = createExtra();
		gameObjects.extra.push_back(newEx);
	}

	//create 3 bats
	if (gameObjects.bat01 == NULL)
		gameObjects.bat01 = createBat(1);
	if (gameObjects.bat02 == NULL)
		gameObjects.bat02 = createBat(2);
	if (gameObjects.bat03 == NULL)
		gameObjects.bat03 = createBat(3);

	if (gameObjects.ghost == NULL)
		gameObjects.ghost = createGhost();

	//reset map with special keys
	for (int i = 0; i < KEYS_COUNT; i++)
		gameState.keyMap[i] = false;
}

// draw scene, set positions
void drawWindowContents()
{
	// static viewpoint - top view
	glm::mat4 orthoViewMatrix = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	if (gameState.cameraSetup == true)
		setupCamera();

	// setup parallel projection ~ left, right, bottom, top, near, far
	glm::mat4 orthoProjectionMatrix = glm::ortho(
		-SCENE_WIDTH, SCENE_WIDTH,
		-SCENE_HEIGHT, SCENE_HEIGHT,
		-10.0f * SCENE_DEPTH, 10.0f * SCENE_DEPTH);

	glm::mat4 projectionMatrix = orthoProjectionMatrix;

	glm::vec3 cameraPosition = gameObjects.camera->position;
	glm::vec3 cameraCenter = gameObjects.camera->position + gameObjects.camera->direction; //bod na ktery kouka
	glm::vec3 cameraUpVector = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 cameraViewDirection = gameObjects.camera->direction;
	glm::vec3 rotationAxis = glm::cross(cameraViewDirection, glm::vec3(0.0f, 0.0f, 1.0f)); //vektorovy soucin, osa podle ktere se rotuje
	glm::mat4 cameraTransform = glm::rotate(glm::mat4(1.0f), -gameState.cameraElevationAngle, rotationAxis); //co, o kolik, podle ktere osy

	cameraUpVector = glm::vec3(cameraTransform * glm::vec4(cameraUpVector, 0.0f)); //transformace up vektoru - musi byt kolmy k smerem pohledu
	cameraViewDirection = glm::vec3(cameraTransform * glm::vec4(cameraViewDirection, 0.0f)); //to same se smerem pohledu
	cameraCenter = cameraPosition + cameraViewDirection; //update pohledu

	//vypocitani matic pri otaceni kamerou
	glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraCenter, cameraUpVector); //bod bod vektor
	projectionMatrix = glm::perspective(60.0f, gameState.windowWidth / (float)gameState.windowHeight, 0.01f, 10.0f);

	glUseProgram(shaderProgram.program);
	//uniforms of shaderProgram
	//lights
	glUniform4fv(shaderProgram.reflectorPositionLocation, 1, glm::value_ptr(glm::vec4(gameObjects.camera->position, 1.0f)));
	glUniform3fv(shaderProgram.reflectorDirectionLocation, 1, glm::value_ptr(cameraViewDirection));
	glUniform1i(shaderProgram.reflectorOnLocation, gameState.reflectorOn);
	glUniform1i(shaderProgram.sunOnLocation, gameState.sunOn);
	glUniform4fv(shaderProgram.pointlightPositionLocation, 1, glm::value_ptr(glm::vec4( gameObjects.ghost->position, 1.0f)) );
	glUniform1i(shaderProgram.pointlightOnLocation, gameState.ghost);

	glUniform1i(shaderProgram.fogOnLocation, gameObjects.fog->fogOn);
	glUniform4fv(shaderProgram.fogColorLocation, 1, glm::value_ptr(gameObjects.fog->color));
	glUniform1f(shaderProgram.fogDensityLocation, gameObjects.fog->density);

	glUseProgram(0);

	glUseProgram(skyboxShaderProgram.program);
	//uniforms of skyboxShaderProgram
	glUniform1i(skyboxShaderProgram.fogOnLocation, gameObjects.fog->fogOn);
	glUniform4fv(skyboxShaderProgram.fogColorLocation, 1, glm::value_ptr(gameObjects.fog->color));
	glUniform1f(skyboxShaderProgram.fogDensityLocation, gameObjects.fog->density);

	glUseProgram(0);

	gameObjects.rain->position = gameObjects.camera->position + cameraViewDirection*0.012f;
	gameObjects.rain->direction = glm::normalize(gameObjects.camera->position - gameObjects.rain->position);

	//draw skybox
	drawSkybox(viewMatrix, projectionMatrix, gameState.sunOn);

	// draw trees
	for (GameObjectsList::iterator it = gameObjects.trees01.begin(); it != gameObjects.trees01.end(); ++it) {
		Object * tree = (Object*)(*it);
		drawTree(tree, viewMatrix, projectionMatrix, 1);
	}

	for (GameObjectsList::iterator it = gameObjects.trees02.begin(); it != gameObjects.trees02.end(); ++it) {
		Object * tree = (Object*)(*it);
		drawTree(tree, viewMatrix, projectionMatrix, 2);
	}

	for (GameObjectsList::iterator it = gameObjects.trees03.begin(); it != gameObjects.trees03.end(); ++it) {
		Object * tree = (Object*)(*it);
		drawTree(tree, viewMatrix, projectionMatrix, 3);
	}

	for (GameObjectsList::iterator it = gameObjects.trees04.begin(); it != gameObjects.trees04.end(); ++it) {
		Object * tree = (Object*)(*it);
		drawTree(tree, viewMatrix, projectionMatrix, 4);
	}

	//draw 3 bats
	drawBat(gameObjects.bat01, viewMatrix, projectionMatrix);
	drawBat(gameObjects.bat02, viewMatrix, projectionMatrix);
	drawBat(gameObjects.bat03, viewMatrix, projectionMatrix);
	
	//draw rock
	drawRock(gameObjects.rock, viewMatrix, projectionMatrix);

	//enable stencil for mouse detection
	glClearStencil(0);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	//draw extra object
	glStencilFunc(GL_ALWAYS, 3, -1);
	for (GameObjectsList::iterator it = gameObjects.extra.begin(); it != gameObjects.extra.end(); ++it) {
		Object * extra = (Object*)(*it);
		drawExtra(extra, viewMatrix, projectionMatrix, gameState.diffColor);
	}

	//draw skull
	glStencilFunc(GL_ALWAYS, 4, -1);
	drawSkull(gameObjects.skull, viewMatrix, projectionMatrix);

	//draw mushroom
	glStencilFunc(GL_ALWAYS, 1, -1);
	drawMushroom(gameObjects.mush, viewMatrix, projectionMatrix);
	
	//draw ground
	glStencilFunc(GL_ALWAYS, 2, -1);
	drawGround(gameObjects.ground, viewMatrix, projectionMatrix);
	glDisable(GL_STENCIL_TEST);

	//ghost
	if (gameState.ghost)
		drawGhost(gameObjects.ghost, viewMatrix, projectionMatrix);
	
	//rain
	if (gameState.rain)
		drawRain(gameObjects.rain, viewMatrix, projectionMatrix);

	//smoke
	glDisable(GL_DEPTH_TEST);
	if (gameObjects.smoke != NULL)
		drawSmoke(gameObjects.smoke, viewMatrix, projectionMatrix);
	glEnable(GL_DEPTH_TEST);
}

// update the display
void displayCallback()
{
	GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
	mask |= GL_STENCIL_BUFFER_BIT;

	glClear(mask);
	drawWindowContents();
	glutSwapBuffers();
}

// window resize ~ pixels
void reshapeCallback(int newWidth, int newHeight)
{
	gameState.windowWidth = newWidth;
	gameState.windowHeight = newHeight;
	glViewport(0, 0, (GLsizei)newWidth, (GLsizei)newHeight);
}

//simulates lightning
void lightning(float elapsedTime)
{
	int time = ((int)(elapsedTime * 1000)) % 7000;
	if (gameState.sunOn)
	{
		if (((time >= 0) && (time < 225)) || ((time >=250) && (time < 400)))
			gameState.sunOn = true;
		else
			gameState.sunOn = false;
	}
	else
	{
		if (((time >= 0) && (time < 225)) || ((time >= 250) && (time < 400)))
			gameState.sunOn = true;
	}
}

// update objects ~ camera: time * speed, rain, bats and ghost depend on time!
void updateObjects(float elapsedTime)
{
	// update camera
	float timeDelta = elapsedTime - gameObjects.camera->currentTime;
	gameObjects.camera->currentTime = elapsedTime;
	if (gameState.freeCameraMode == true) 
	{
		// move forward (W)
		if (gameState.keyMap[UP] == true) 
		{
			glm::vec3 newPosition = gameObjects.camera->position + CAMERA_MOVEMENT_SPEED * timeDelta * gameObjects.camera->direction;
			//move only in the scene!
			if ((fabs(newPosition.x) < SCENE_WIDTH) && (fabs(newPosition.y) < SCENE_HEIGHT))
			{
				//check for collision with other objects
				if (gameState.cameraNumber == 0)
				{
					if (!isCollision(newPosition))
						gameObjects.camera->position = newPosition;
				}
				else
					gameObjects.camera->position = newPosition;
			}
		}

		//move backward (S)
		if (gameState.keyMap[DOWN] == true) 
		{
			glm::vec3 newPosition = gameObjects.camera->position - CAMERA_MOVEMENT_SPEED * timeDelta * gameObjects.camera->direction;
			if ((fabs(newPosition.x) < SCENE_WIDTH) && (fabs(newPosition.y) < SCENE_HEIGHT))
			{
				if (gameState.cameraNumber == 0)
				{
					if (!isCollision(newPosition))
						gameObjects.camera->position = newPosition;
				}
				else
					gameObjects.camera->position = newPosition;
			}
		}

		if (gameState.keyMap[RIGHT] == true)
			turnCameraRight(VIEW_ANGLE_DELTA);
		if (gameState.keyMap[LEFT] == true)
			turnCameraLeft(VIEW_ANGLE_DELTA);
	}

	//update bats
	gameObjects.bat01->currentTime = elapsedTime;
	float bat01CurveParamT = gameObjects.bat01->speed * (gameObjects.bat01->currentTime - gameObjects.bat01->startTime);
	gameObjects.bat01->position = evaluateClosedCurve(bat01CurveData, bat01CurveSize, bat01CurveParamT); //+ gameObjects.bat->initPosition;
	gameObjects.bat01->direction = glm::normalize(evaluateClosedCurve_1stDerivative(bat01CurveData, bat01CurveSize, bat01CurveParamT));
	
	gameObjects.bat02->currentTime = elapsedTime;
	float bat02CurveParamT = gameObjects.bat02->speed * (gameObjects.bat02->currentTime - gameObjects.bat02->startTime);
	gameObjects.bat02->position = evaluateClosedCurve(bat02CurveData, bat02CurveSize, bat02CurveParamT); 
	gameObjects.bat02->direction = glm::normalize(evaluateClosedCurve_1stDerivative(bat02CurveData, bat02CurveSize, bat02CurveParamT));
	
	gameObjects.bat03->currentTime = elapsedTime;
	float bat03CurveParamT = gameObjects.bat03->speed * (gameObjects.bat03->currentTime - gameObjects.bat03->startTime);
	gameObjects.bat03->position = evaluateClosedCurve(bat03CurveData, bat03CurveSize, bat03CurveParamT);
	gameObjects.bat03->direction = glm::normalize(evaluateClosedCurve_1stDerivative(bat03CurveData, bat03CurveSize, bat03CurveParamT));

	//update ghost
	gameObjects.ghost->currentTime = elapsedTime;
	float ghostCurveParamT = gameObjects.ghost->speed * (gameObjects.ghost->currentTime - gameObjects.ghost->startTime);
	gameObjects.ghost->position = evaluateClosedCurve(ghostCurveData, ghostCurveSize, ghostCurveParamT);
	gameObjects.ghost->direction = glm::normalize(evaluateClosedCurve_1stDerivative(ghostCurveData, ghostCurveSize, ghostCurveParamT));

	//DO NOT FORGET UPDATE RAIN!!!!
	gameObjects.rain->currentTime = elapsedTime;
	if ((!gameState.sunForced) && gameState.rain)
		lightning(elapsedTime);

	//update smoke
	if (gameObjects.smoke != NULL)
	{
		gameObjects.smoke->currentTime = elapsedTime;
		if ( (gameObjects.smoke->currentTime) > (gameObjects.smoke->startTime + gameObjects.smoke->texFrames * gameObjects.smoke->frameDuration) )
			gameObjects.smoke = NULL;
	}
}

// update scene time
void timerCallback(int)
{
	gameState.elapsedTime = 0.001f * (float)glutGet(GLUT_ELAPSED_TIME); // milliseconds => seconds
	updateObjects(gameState.elapsedTime); // update objects in the scene

	glutTimerFunc(33, timerCallback, 0);
	glutPostRedisplay();
}

// mouse moving ~ turn camera left/right
void passiveMouseMotionCallback(int mouseX, int mouseY)
{
	//mouse has to always be in the center of window
	if (mouseY != gameState.windowHeight / 2) {

		float cameraElevationAngleDelta = 0.5f * (mouseY - gameState.windowHeight / 2);
		
		if (fabs(gameState.cameraElevationAngle + cameraElevationAngleDelta) < CAMERA_ELEVATION_MAX) //nesmi prejit pravy uhel
			gameState.cameraElevationAngle += cameraElevationAngleDelta;
		
		glutWarpPointer(gameState.windowWidth / 2, gameState.windowHeight / 2);
		glutPostRedisplay();
	}
	//rotates to left, right ~ cyclic
	if (mouseX != gameState.windowWidth / 2) {
		
		float cameraTurnAngleDelta = 0.5f * (mouseX - gameState.windowWidth / 2);
		
		if ((mouseX - gameState.windowWidth / 2) < 0)
			turnCameraLeft(-cameraTurnAngleDelta);
		else
			turnCameraRight(cameraTurnAngleDelta);
		
		glutWarpPointer(gameState.windowWidth / 2, gameState.windowHeight / 2);
		glutPostRedisplay();
	}
}

// key pressed ~ 27: call glutLeaveMainLoop() to exit the program
void keyboardCallback(unsigned char keyPressed, int mouseX, int mouseY)
{
	switch (keyPressed) {
	case 27: //ESC (ASCII value 27)
		glutLeaveMainLoop();
		break;
	case 'f':
		gameState.freeCameraMode = !gameState.freeCameraMode;
		if (gameState.freeCameraMode == true) {
			glutPassiveMotionFunc(passiveMouseMotionCallback);
			glutWarpPointer(gameState.windowWidth / 2, gameState.windowHeight / 2);
		}
		else {
			gameState.cameraSetup = true;
			glutPassiveMotionFunc(NULL);
		}
		break;
	
	// switch (C)amera
	case 'c':
		gameState.cameraNumber++;
		gameState.cameraNumber %= 3;
		gameState.cameraSetup = true;
		gameState.freeCameraMode = false;
		glutPassiveMotionFunc(NULL);
		break;

	// fo(G) on/off
	case 'g':
		gameObjects.fog->fogOn = !gameObjects.fog->fogOn;
		break;

	//(R)ain on/off
	case 'r':
		gameState.rain = !gameState.rain;
		break;

	//sun (O)n/off
	case 'o':
		gameState.sunOn = !gameState.sunOn;
		gameState.sunForced = !gameState.sunForced;
		break;

	//(B)attery on/off  
	case 'b':
		gameState.reflectorOn = !gameState.reflectorOn;
		break;

	// move forward
	case 'w':
		gameState.keyMap[UP] = true;
		break;
	// turn left
	case 'a':
		gameState.keyMap[LEFT] = true;
		break;
	// move backward
	case 's':
		gameState.keyMap[DOWN] = true;
		break;
	// turn right
	case 'd':
		gameState.keyMap[RIGHT] = true;
		break;
	default:
		break;
	}
}

// key release
void keyboardUpCallback(unsigned char keyPressed, int mouseX, int mouseY)
{
	switch (keyPressed) {
	case 'w':
		gameState.keyMap[UP] = false;
		break;
	case 'a':
		gameState.keyMap[LEFT] = false;
		break;
	case 's':
		gameState.keyMap[DOWN] = false;
		break;
	case 'd':
		gameState.keyMap[RIGHT] = false;
		break;
	default:;
	}
}

// special key pressed - ghost appears, restart
void specialKeyboardCallback(int specKeyPressed, int mouseX, int mouseY)
{
	if (specKeyPressed == GLUT_KEY_F1) gameState.ghost = !gameState.ghost;
	if (specKeyPressed == GLUT_KEY_F2) restart();
}

// reaction on menu item
void menu(int choice)
{
	switch (choice) {
	case 1:
		restart();
		break;
	case 2:
		gameState.cameraNumber = 0;
		gameState.cameraSetup = true;
		gameState.freeCameraMode = false;
		glutPassiveMotionFunc(NULL);
		break;
	case 3:
		gameState.cameraNumber = 1;
		gameState.cameraSetup = true;
		gameState.freeCameraMode = false;
		glutPassiveMotionFunc(NULL);
		break;
	case 4:
		gameState.cameraNumber = 2;
		gameState.cameraSetup = true;
		gameState.freeCameraMode = false;
		glutPassiveMotionFunc(NULL);
		break;
	case 5:
		gameState.freeCameraMode = !gameState.freeCameraMode;
		if (gameState.freeCameraMode == true) {
			glutPassiveMotionFunc(passiveMouseMotionCallback);
			glutWarpPointer(gameState.windowWidth / 2, gameState.windowHeight / 2);
		}
		else {
			gameState.cameraSetup = true;
			glutPassiveMotionFunc(NULL);
		}
		break;
	case 6:
		gameState.sunOn = true;
		gameState.sunForced = true;
		break;
	case 7:
		gameState.sunOn = false;
		gameState.sunForced = false;
		break;
	case 8:
		gameState.reflectorOn = true;
		break;
	case 9:
		gameState.reflectorOn = false;
		break;
	case 10:
		gameObjects.fog->fogOn = true;
		break;
	case 11:
		gameObjects.fog->fogOn = false;
		break;
	case 12:
		gameState.rain = true;
		break;
	case 13:
		gameState.rain = false;
		break;
	case 14: 
		exit(0);
		break;
	}
	glutPostRedisplay();
}

// create menu
void createMenu(void)
{
	int submenuCamera = glutCreateMenu(menu);
	glutAddMenuEntry("Static (on ground)", 2);
	glutAddMenuEntry("Static (above) 1", 3);
	glutAddMenuEntry("Static (above) 2", 4);
	glutAddMenuEntry("Free", 5);

	int submenuSun = glutCreateMenu(menu);
	glutAddMenuEntry("On", 6);
	glutAddMenuEntry("Off", 7);

	int submenuRain = glutCreateMenu(menu);
	glutAddMenuEntry("On", 12);
	glutAddMenuEntry("Off", 13);

	int submenuFog = glutCreateMenu(menu);
	glutAddMenuEntry("On", 10);
	glutAddMenuEntry("Off", 11);

	int submenuReflector = glutCreateMenu(menu);
	glutAddMenuEntry("On", 8);
	glutAddMenuEntry("Off", 9);

	glutCreateMenu(menu);
	glutAddMenuEntry("Restart", 1);
	glutAddSubMenu("Camera", submenuCamera);
	glutAddSubMenu("Moonlight", submenuSun);
	glutAddSubMenu("Rain", submenuRain);
	glutAddSubMenu("Fog", submenuFog);
	glutAddSubMenu("Flashlight", submenuReflector);
	glutAddMenuEntry("Quit", 14);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// mouse is clicked ~ reads from stencil buffer
void mouseCallback(int buttonPressed, int buttonState, int mouseX, int mouseY)
{
	if ((buttonPressed == GLUT_LEFT_BUTTON) && (buttonState == GLUT_DOWN)) {
		// stores value from the stencil buffer (byte)
		unsigned char objectID = 0;
		int y = gameState.windowHeight - mouseY - 1; //otocene Y [0,0] v levem hornim rohu (jeste -1!!!!)
		glReadPixels(mouseX, y, 1, 1, GL_STENCIL_INDEX, GL_BYTE, &objectID);
		//GLfloat depth = 0.0f;
		//glReadPixels(mouseX, mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

		int res;

		switch (objectID)
		{
		case 1: //mushroom
			gameState.attemptCnt++;
			std::cout << "You've found mushroom but it got away!" << std::endl;
			std::cout << "# of attempt: " << gameState.attemptCnt << std::endl;
			gameObjects.mush->position = generateRandomPosition(1, 0, false);
			do 	//generate in reachable area
			{
				gameObjects.mush->position = generateRandomPosition(1, 0, false);
			} while (fabs(gameObjects.mush->position.x) >= SCENE_WIDTH || fabs(gameObjects.mush->position.y) >= SCENE_HEIGHT);
			gameObjects.mush->position.z = -0.23f;
			break;
		case 2: //ground
			std::cout << "You've clicked on this particular place." << std::endl;
			break;
		case 3: //extra object
			gameState.diffColor = !gameState.diffColor;
			break;
		case 4: //skull, spawns a ghost if clicked
			gameState.ghost = !gameState.ghost;
			if (gameObjects.smoke == NULL)
			{
				glm::vec3 smokePosition = gameObjects.skull->position;
				smokePosition.z = 0.0f;
				gameObjects.smoke = createSmoke(smokePosition);
			}
			break;
		default:
			break;
		} //end switch
	} //end if
}

// inicialize whole application
void initializeApplication()
{
	// initialize random seed
	srand((unsigned int)time(NULL));

	// initialize OpenGL
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClearStencil(0);

	// initialize shaders
	initializeShaderPrograms();
	// create geometry for all models used
	initializeModels();

	gameObjects.fog = NULL;
	gameObjects.skull = NULL;
	gameObjects.mush = NULL;
	gameObjects.camera = NULL;
	gameObjects.bat01 = NULL;
	gameObjects.bat02 = NULL;
	gameObjects.bat03 = NULL;
	gameObjects.ghost = NULL;
	gameObjects.smoke = NULL;

	gameState.sunOn = false;
	gameState.reflectorOn = false;
	restart();
}

// finalize whole application
void finalizeApplication(void)
{
	cleanUpObjects();
	delete gameObjects.camera;
	gameObjects.camera = NULL;

	// delete buffers 
	clearModels();

	// delete shaders
	cleanupShaderPrograms();
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	// initialize windowing system
	glutInit(&argc, argv);

	glutInitContextVersion(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

	// initial window size
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow(WINDOW_TITLE);
	//glutPositionWindow(0, 0);

	glutDisplayFunc(displayCallback);
	// register callback for change of window size
	glutReshapeFunc(reshapeCallback);
	// register callbacks for keyboard
	glutKeyboardFunc(keyboardCallback);
	glutKeyboardUpFunc(keyboardUpCallback);
	glutSpecialFunc(specialKeyboardCallback);
	glutMouseFunc(mouseCallback);
	createMenu();
	glutTimerFunc(33, timerCallback, 0);

	// initialize GL
	if (!pgr::initialize(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR))
		pgr::dieWithError("pgr init failed, required OpenGL not supported?");

	initializeApplication();
	glutCloseFunc(finalizeApplication);
	glutMainLoop();

	return 0;
}
