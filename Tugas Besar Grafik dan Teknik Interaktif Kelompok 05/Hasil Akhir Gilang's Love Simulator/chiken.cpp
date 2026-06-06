#include "chiken.h"
#include "global.h"
#include "utils.h"
#include <GL/glut.h>
#include <cmath>

static const float PI    = 3.14159265f;
static const float D2R   = PI / 180.0f;

// Batas kandang ayam (sesuai mainApanih)
static const float CH_XMIN = CHICKEN_CX - 8.0f*0.5f + 0.4f;
static const float CH_XMAX = CHICKEN_CX + 8.0f*0.5f - 0.4f;
static const float CH_ZMIN = CHICKEN_CZ - 8.0f*0.5f + 0.4f;
static const float CH_ZMAX = CHICKEN_CZ + 8.0f*0.5f - 0.4f;

// RNG sederhana (tidak pakai rand() supaya deterministik)
static unsigned int chickenRandState = 12345;
static unsigned int chickenRand() {
    chickenRandState ^= chickenRandState << 13;
    chickenRandState ^= chickenRandState >> 17;
    chickenRandState ^= chickenRandState << 5;
    return chickenRandState;
}

void initChickens() {
    float startPos[NUM_CHICKENS][2] = {
        {32.0f, -35.0f},
        {30.5f, -33.5f},
        {33.5f, -33.5f},
        {30.5f, -36.5f},
        {33.5f, -36.5f},
    };
    for(int i = 0; i < NUM_CHICKENS; i++) {
        chickens[i].x           = startPos[i][0];
        chickens[i].z           = startPos[i][1];
        chickens[i].angle       = (float)(i * 72);
        chickens[i].legAnim     = (float)i * 0.5f;
        chickens[i].speed       = 0.07f + 0.03f * i;
        chickens[i].turnTimer   = 30 + i * 20;
        chickens[i].turnTarget  = chickens[i].angle;
    }
}

void updateChickens() {
    for(int i = 0; i < NUM_CHICKENS; i++) {
        Chicken &ch = chickens[i];
        ch.turnTimer--;
        if(ch.turnTimer <= 0) {
            float randAngle = (float)(chickenRand() % 360);
            ch.turnTarget = randAngle;
            ch.turnTimer  = 20 + (int)(chickenRand() % 40);
        }

        float diff = ch.turnTarget - ch.angle;
        while(diff >  180.0f) diff -= 360.0f;
        while(diff < -180.0f) diff += 360.0f;
        ch.angle += diff * 0.08f;

        float rad = ch.angle * D2R;
        float nx = ch.x + sinf(rad) * ch.speed;
        float nz = ch.z + cosf(rad) * ch.speed;

        if(nx < CH_XMIN || nx > CH_XMAX || nz < CH_ZMIN || nz > CH_ZMAX) {
            ch.turnTarget = ch.angle + 90.0f + (float)(chickenRand() % 90);
        } else {
            ch.x = nx;
            ch.z = nz;
            ch.legAnim += 0.25f;
        }
    }
}

void drawSingleChicken(float x, float z, float angle, float legPhase) {
    glPushMatrix();
        glTranslatef(x, 0.4f, z);
        glRotatef(angle, 0, 1, 0);
        glScalef(0.5f, 0.5f, 0.5f);

        // Badan
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
            glScalef(0.8f, 0.9f, 1.1f);
            glutSolidSphere(0.5, 20, 20);
        glPopMatrix();

        // Kepala
        glPushMatrix();
            glTranslatef(0, 0.4f, 0.4f);
            glutSolidSphere(0.35, 20, 20);
            // Paruh
            glColor3f(1.0f, 0.5f, 0.0f);
            glPushMatrix();
                glTranslatef(0, -0.05f, 0.35f);
                glScalef(1, 0.8f, 1.5f);
                glutSolidCone(0.1, 0.2, 10, 10);
            glPopMatrix();
        glPopMatrix();

        // Kaki
        float s = sinf(legPhase) * 30.0f;
        glColor3f(1.0f, 0.8f, 0.0f);
        glPushMatrix();
            glTranslatef(0.2f, -0.4f, 0);
            glRotatef(s, 1, 0, 0);
            glScalef(0.1f, 0.6f, 0.1f);
            glutSolidCube(1.0);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(-0.2f, -0.4f, 0);
            glRotatef(-s, 1, 0, 0);
            glScalef(0.1f, 0.6f, 0.1f);
            glutSolidCube(1.0);
        glPopMatrix();
    glPopMatrix();
}
