#include "utils.h"
#include <GL/glut.h>
#include <cstdlib>
#include <cmath>

float rnd(float lo, float hi) {
    return lo + (hi - lo) * ((float)rand() / (float)RAND_MAX);
}

float clampF(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

Color3 makeColor(float r, float g, float b) {
    Color3 c; c.r=r; c.g=g; c.b=b; return c;
}

Color3 shade(Color3 c, float f) {
    return makeColor(clampF(c.r*f,0,1), clampF(c.g*f,0,1), clampF(c.b*f,0,1));
}

void setColor(float r, float g, float b) { glColor3f(r,g,b); }
void setColor(Color3 c)                  { glColor3f(c.r,c.g,c.b); }

void autoNormal(float x1,float y1,float z1,
                float x2,float y2,float z2,
                float x3,float y3,float z3) {
    float ux=x2-x1, uy=y2-y1, uz=z2-z1;
    float vx=x3-x1, vy=y3-y1, vz=z3-z1;
    float nx=uy*vz-uz*vy, ny=uz*vx-ux*vz, nz=ux*vy-uy*vx;
    float len=sqrtf(nx*nx+ny*ny+nz*nz);
    if(len>0){nx/=len;ny/=len;nz/=len;}
    glNormal3f(nx,ny,nz);
}

void quad4(float x1,float y1,float z1,
           float x2,float y2,float z2,
           float x3,float y3,float z3,
           float x4,float y4,float z4) {
    autoNormal(x1,y1,z1,x2,y2,z2,x3,y3,z3);
    glBegin(GL_QUADS);
        glVertex3f(x1,y1,z1); glVertex3f(x2,y2,z2);
        glVertex3f(x3,y3,z3); glVertex3f(x4,y4,z4);
    glEnd();
}

void tri3(float x1,float y1,float z1,
          float x2,float y2,float z2,
          float x3,float y3,float z3) {
    autoNormal(x1,y1,z1,x2,y2,z2,x3,y3,z3);
    glBegin(GL_TRIANGLES);
        glVertex3f(x1,y1,z1); glVertex3f(x2,y2,z2); glVertex3f(x3,y3,z3);
    glEnd();
}
