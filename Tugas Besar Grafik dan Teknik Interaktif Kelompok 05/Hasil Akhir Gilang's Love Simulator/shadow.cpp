#include "shadow.h"
#include "global.h"
#include <GL/glut.h>
#include <cmath>

static const float SHADOW_LIGHT[4] = { -15.0f, 12.0f, 10.0f, 0.0f };

// Bangun shadow matrix (planar shadow ke y=0)
static void buildShadowMatrix(float m[16], const float light[4]) {
    float lx = light[0], ly = light[1], lz = light[2], lw = light[3];
    float dot = ly; // normal lantai (0,1,0), d=0 => dot = ly
    m[ 0]=dot-lx*0; m[ 4]=   -lx*1; m[ 8]=   -lx*0; m[12]=   -lx*0;
    m[ 1]=   -ly*0; m[ 5]=dot-ly*1; m[ 9]=   -ly*0; m[13]=   -ly*0;
    m[ 2]=   -lz*0; m[ 6]=   -lz*1; m[10]=dot-lz*0; m[14]=   -lz*0;
    m[ 3]=   -lw*0; m[ 7]=   -lw*1; m[11]=   -lw*0; m[15]=dot-lw*0;
}

static void beginShadow() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.35f);
}

static void endShadow() {
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void drawPlayerShadow() {
    float m[16];
    buildShadowMatrix(m, SHADOW_LIGHT);
    beginShadow();
    glPushMatrix(); glMultMatrixf(m);
    glPushMatrix(); glTranslatef(playerX, playerY+1.2f, playerZ); glRotatef(playerYaw,0,1,0);
    // Badan
    glPushMatrix(); glScalef(0.75f,0.95f,0.38f); glutSolidCube(1.0f); glPopMatrix();
    // Kepala
    glPushMatrix(); glTranslatef(0,0.78f,0); glScalef(0.58f,0.58f,0.54f); glutSolidCube(1.0f); glPopMatrix();
    // Kaki Kiri
    glPushMatrix(); glTranslatef(-0.18f,-0.5f,0); glRotatef(-swingAngle,1,0,0);
        glPushMatrix(); glTranslatef(0,-0.55f,0); glScalef(0.30f,0.85f,0.30f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();
    // Kaki Kanan
    glPushMatrix(); glTranslatef(0.18f,-0.5f,0); glRotatef(swingAngle,1,0,0);
        glPushMatrix(); glTranslatef(0,-0.55f,0); glScalef(0.30f,0.85f,0.30f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();
    // Tangan Kiri
    glPushMatrix(); glTranslatef(-0.48f,0.30f,0); glRotatef(swingAngle,1,0,0);
        glPushMatrix(); glTranslatef(0,-0.30f,0); glScalef(0.22f,0.60f,0.22f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();
    // Tangan Kanan
    glPushMatrix(); glTranslatef(0.48f,0.30f,0); glRotatef(-swingAngle,1,0,0);
        glPushMatrix(); glTranslatef(0,-0.30f,0); glScalef(0.22f,0.60f,0.22f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();
    glPopMatrix(); glPopMatrix();
    endShadow();
}

void drawTisyaShadow() {
    float m[16];
    buildShadowMatrix(m, SHADOW_LIGHT);
    beginShadow();
    glPushMatrix(); glMultMatrixf(m);
    glPushMatrix(); glTranslatef(tisyaX, 1.2f, tisyaZ); glRotatef(tisyaYaw,0,1,0);
    glPushMatrix(); glScalef(0.70f,0.90f,0.36f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0,0.78f,0); glScalef(0.56f,0.56f,0.52f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.14f,-0.55f,0); glScalef(0.28f,0.85f,0.28f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.14f,-0.55f,0); glScalef(0.28f,0.85f,0.28f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix(); glPopMatrix();
    endShadow();
}

void drawCowShadow() {
    float m[16];
    buildShadowMatrix(m, SHADOW_LIGHT);
    beginShadow();
    glPushMatrix(); glMultMatrixf(m);
    glPushMatrix(); glTranslatef(cowX, 0.f, cowZ); glRotatef(cowAngle,0,1,0);
    glTranslatef(0, 1.3f, 0);
    glPushMatrix(); glScalef(0.85f,0.85f,1.4f); glutSolidSphere(1.f,10,7); glPopMatrix();
    glPopMatrix(); glPopMatrix();
    endShadow();
}

void drawChickenShadow(float x, float z, float angle) {
    float m[16];
    buildShadowMatrix(m, SHADOW_LIGHT);
    beginShadow();
    glPushMatrix(); glMultMatrixf(m);
    glPushMatrix(); glTranslatef(x, 0.4f, z); glRotatef(angle,0,1,0); glScalef(0.5f,0.5f,0.5f);
    glPushMatrix(); glScalef(0.8f,0.9f,1.1f); glutSolidSphere(0.5f,8,6); glPopMatrix();
    glPushMatrix(); glTranslatef(0,0.4f,0.4f); glutSolidSphere(0.35f,8,6); glPopMatrix();
    glPopMatrix(); glPopMatrix();
    endShadow();
}
