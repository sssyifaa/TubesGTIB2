#ifndef UTILS_H
#define UTILS_H

#include "global.h"

float   rnd(float lo, float hi);
float   clampF(float v, float lo, float hi);
Color3  makeColor(float r, float g, float b);
Color3  shade(Color3 c, float f);
void    setColor(float r, float g, float b);
void    setColor(Color3 c);
void    autoNormal(float x1,float y1,float z1,
                   float x2,float y2,float z2,
                   float x3,float y3,float z3);
void    quad4(float x1,float y1,float z1,
              float x2,float y2,float z2,
              float x3,float y3,float z3,
              float x4,float y4,float z4);
void    tri3(float x1,float y1,float z1,
             float x2,float y2,float z2,
             float x3,float y3,float z3);

#endif
