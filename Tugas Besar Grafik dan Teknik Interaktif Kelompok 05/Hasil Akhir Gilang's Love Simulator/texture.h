#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glut.h>

extern GLuint texGrass;
extern GLuint texDirt;
extern GLuint texRoad;
extern GLuint texWood;

// Fungsi tekstur (sama persis dengan mainApanih)
void generateTextures();
void enableTex(GLuint id);
void disableTex();

#endif
