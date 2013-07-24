#include "stdafx.h"

//CUSTOM BUILD OPTIONS
#define EDGE_STIFFNESS 1.0f
#define TORSION_STIFFNESS 0.1f
#define BENDING_STIFFNESS 0.01f
#define ANCHOR_STIFFNESS 0.01f
#define INITIAL_ORIENTATION Ogre::Quaternion(Ogre::Radian(Ogre::Degree(0)),Ogre::Vector3(0,0,1))
#define ANIMATION_SPEED 0.4
#define HAIR_QUADRATIC_A -13.9
#define HAIR_QUADRATIC_B 4.9
#define HAIR_QUADRATIC_C 6.4
//#define SHOW_BONES
#define DEBUG_VISUALISATION
#define VARIABLE_SILHOUETTE_INTENSITY
#define OUTER_SILHOUETTE
//#define IMAGESPACE_SILHOUETTE
#define SHOW_NORMALS
//TYPES OF SPRINGS IN THIS BUILD
#define EDGE_SPRINGS
#define BENDING_SPRINGS
#define TORSION_SPRINGS
#define ANCHOR_SPRINGS
#define GHOST_STRAND
//TYPES OF STROKE QUAD SCALING
#define ANGLE_SCALING
#define DEPTH_SCALING
#define STROKE_SCALE 0.02f
#define STROKE_LIMIT 2.0f
//ANCHOR BLENDING OPTIONS
//#define CONSTANT_BLENDING_SPRINGS
#define BLENDING_QUADRATIC_A 1.0
#define BLENDING_QUADRATIC_B 0
#define BLENDING_QUADRATIC_C 0
//HATCHING OPTIONS
//#define IMAGE_SPACE_HATCHING
//ID BUFFER COLOUR INCREMENT BETWEEN NEW IDS
#define ID_INCREMENT 0.05f
//ANCHOR ANIMATION SPEED SCALE
#define ANCHOR_FRAME_INCRMENT 0.01f
//HAIR STRAND SETTINGS
#define NUM_HAIR_SAMPLES 8
#define NUM_HAIR_SHAPE_SAMPLES 8
//BULLET PHYSICS collision groups used to limit collisions between certain groups
#define BODY_GROUP 0x1
#define HAIR_GROUP 0x2
#define GHOST_GROUP 0x4
//based on http://bulletphysics.org/mediawiki-1.5.8/index.php/Stepping_The_World
#define FIXED_TIMESTEP btScalar(1.0f)/btScalar(60.0f);
#define MAX_SUB_STEPS 1
//SPECULAR HIGHLIGHTS settings
#define BLINN_SPECULAR
#define SPECULAR_TEXTURE
//#define STYLISED_SPECULAR
#define SPECULAR_WEIGHT 0.3f
#define SHININESS 1.0f
#define KS 1.0f
#define SPECULAR_THRESHOLD 25.0f
#define MAX_MERGING_DISTANCE 2.0f
#define MIN_MERGING_DISTANCE 1.0f
#define MAX_GROUP_SECTIONS 100
#define TIP_EXCLUDE 0.4
#define HIGHLIGHT_SCALE 0.04f
#define MAX_GROUP_SIZE 5
#define MIN_GROUP_SIZE 2
//VECTOR RESERVE SIZES
#define CANDIDATES_RESERVE_SIZE 100
#define TEMP_SILHOUETTE_RESERVE_SIZE 40

