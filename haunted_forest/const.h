//----------------------------------------------------------------------------------------
/**
*      file	|		const.h
*/
//----------------------------------------------------------------------------------------
#ifndef __CONST_H
#define __CONST_H

#define WIDTH 1200
#define HEIGHT 800
#define WINDOW_TITLE "Haunted forest (BI-PGR)"

// size of scene
#define SCENE_WIDTH 3.0f
#define SCENE_HEIGHT 3.0f
#define SCENE_DEPTH 1.0f

// map of keys
enum { LEFT, RIGHT, UP, DOWN, KEYS_COUNT };

//textures
#define GROUND_TEXTURE "data/ground/groundtexture3.jpg"
#define ROCK_TEXTURE "data/stone/rock_base_color.png"
#define RAIN_TEXTURE "data/rain-png-transparent-11.png"
#define SKYBOX_CUBE_TEXTURE_FILE_PREFIX_DAY "data/skybox/skybox"
#define SKYBOX_CUBE_TEXTURE_FILE_PREFIX_NIGHT "data/skybox/skybox2"
#define SMOKE_TEXTURE "data/smoke2.png"

// sources for objects
#define TREE_MODEL_01 "data/trees/dead_tree_01.obj"
#define TREE_MODEL_02 "data/trees/dead_tree_02.obj"
#define TREE_MODEL_03 "data/trees/dead_tree_03.obj"
#define TREE_MODEL_04 "data/trees/dead_tree_04.obj"
#define EXTRA_OBJECT_MODEL "data/mush/mush.obj"
#define EXTRANEG_OBJECT_MODEL "data/mush/mushneg.obj"
#define SKULL_MODEL "data/skull/skull.obj"
#define MUSHROOM_MODEL "data/mushroom/Mushroom2.obj"
#define BAT_MODEL "data/bat/bat.obj"
#define GHOST_MODEL "data/ghost/ghost.obj"

// number of objects in scene
#define TREES01_COUNT 15
#define TREES02_COUNT 23
#define TREES03_COUNT 19
#define TREES04_COUNT 20
#define EXTRA_OBJECT_COUNT 5

// delta of view angle
#define VIEW_ANGLE_DELTA 5.0f
// maximal elevation of camera
#define CAMERA_ELEVATION_MAX 50.0f
// movement speed
#define CAMERA_MOVEMENT_SPEED 0.5f

// size of objects in scene
#define TREE_SIZE 0.5f
#define SKULL_SIZE 0.02f
#define MUSH_SIZE 0.05f
#define RAIN_SIZE 0.01f
#define ROCK_SIZE 0.005f
#define EXTRA_OBJECT_SIZE 0.06f
#define GHOST_SIZE 0.15f
#define BAT_SIZE1 0.25f
#define BAT_SIZE2 0.35f
#define BAT_SIZE3 0.50f
#define SMOKE_SIZE 0.1f

#define BAT_SPEED1 2.1f
#define BAT_SPEED2 3.8f
#define BAT_SPEED3 4.0f
#define GHOST_SPEED 0.1f

// misc
#define FOG_DENSITY 1.0f;
#define TRESHOLD_RADIUS 0.13f

#endif // __CONST_H
