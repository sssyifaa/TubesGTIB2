#include "cow.h"
#include "global.h"
#include "mission.h"
#include <GL/glut.h>
#include <cmath>

static const float PI = 3.14159265f;

// Kandang sapi
static const float COW_KANDANG_CX = 22.0f;
static const float COW_KANDANG_CZ = -35.0f;
static const float COW_KANDANG_W  = 8.0f;
static const float COW_KANDANG_D  = 8.0f;
static const float COW_MARGIN     = 1.4f;

static Waypoint cowWaypoints[] = {
    { COW_KANDANG_CX - COW_KANDANG_W*0.5f + COW_MARGIN,
      COW_KANDANG_CZ - COW_KANDANG_D*0.5f + COW_MARGIN },
    { COW_KANDANG_CX + COW_KANDANG_W*0.5f - COW_MARGIN,
      COW_KANDANG_CZ - COW_KANDANG_D*0.5f + COW_MARGIN },
    { COW_KANDANG_CX + COW_KANDANG_W*0.5f - COW_MARGIN,
      COW_KANDANG_CZ + COW_KANDANG_D*0.5f - COW_MARGIN },
    { COW_KANDANG_CX - COW_KANDANG_W*0.5f + COW_MARGIN,
      COW_KANDANG_CZ + COW_KANDANG_D*0.5f - COW_MARGIN },
};
static int   cowWaypointCount   = 4;
static int   cowCurrentWaypoint = 0;
static float cowMoveSpeed       = 0.025f;

void initCow() {
    cowX = cowWaypoints[0].x;
    cowZ = cowWaypoints[0].z;
    cowCurrentWaypoint = 1;
    cowIsMoving = true;
}

// ---- Helper render ----

static float cylX_(float r, float a) { return r * cosf(a); }
static float cylZ_(float r, float a) { return r * sinf(a); }

static void drawCowHead() {
    GLUquadric* q = gluNewQuadric();
    glColor3f(0.92f,0.88f,0.82f);
    glPushMatrix(); glScalef(0.55f,0.5f,0.65f); glutSolidSphere(1.f,16,12); glPopMatrix();
    glColor3f(0.98f,0.85f,0.8f);
    glPushMatrix(); glTranslatef(0.f,-0.15f,0.6f); glScalef(0.35f,0.28f,0.3f); glutSolidSphere(1.f,12,8); glPopMatrix();
    glColor3f(0.4f,0.2f,0.2f);
    glPushMatrix(); glTranslatef(-0.1f,-0.18f,0.85f); glScalef(0.07f,0.05f,0.04f); glutSolidSphere(1.f,8,6); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.1f,-0.18f,0.85f); glScalef(0.07f,0.05f,0.04f); glutSolidSphere(1.f,8,6); glPopMatrix();
    glColor3f(0.1f,0.05f,0.0f);
    glPushMatrix(); glTranslatef(-0.3f,0.12f,0.5f); glutSolidSphere(0.07f,8,6); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.3f,0.12f,0.5f); glutSolidSphere(0.07f,8,6); glPopMatrix();
    glColor3f(1.f,1.f,1.f);
    glPushMatrix(); glTranslatef(-0.27f,0.15f,0.55f); glutSolidSphere(0.025f,6,4); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.33f,0.15f,0.55f); glutSolidSphere(0.025f,6,4); glPopMatrix();
    glColor3f(0.88f,0.78f,0.72f);
    glPushMatrix(); glTranslatef(-0.55f,0.2f,0.1f); glRotatef(-30.f,0,0,1); glScalef(0.18f,0.32f,0.12f); glutSolidSphere(1.f,8,6); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.55f,0.2f,0.1f); glRotatef( 30.f,0,0,1); glScalef(0.18f,0.32f,0.12f); glutSolidSphere(1.f,8,6); glPopMatrix();
    glColor3f(0.75f,0.65f,0.4f);
    glPushMatrix(); glTranslatef(-0.3f,0.45f,0.f); glRotatef(20.f,0,0,1); glRotatef(-15.f,1,0,0); glRotatef(-90.f,1,0,0); gluCylinder(q,0.06f,0.01f,0.5f,8,4); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.3f,0.45f,0.f); glRotatef(-20.f,0,0,1); glRotatef(-15.f,1,0,0); glRotatef(-90.f,1,0,0); gluCylinder(q,0.06f,0.01f,0.5f,8,4); glPopMatrix();
    gluDeleteQuadric(q);
}

static void drawCowBody() {
    glColor3f(0.92f,0.88f,0.82f);
    glPushMatrix(); glScalef(0.85f,0.85f,1.4f); glutSolidSphere(1.f,20,14); glPopMatrix();
    glColor3f(0.15f,0.12f,0.1f);
    glPushMatrix(); glTranslatef( 0.4f,0.3f, 0.2f); glScalef(0.35f,0.28f,0.45f); glutSolidSphere(1.f,10,8); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.5f,0.1f,-0.4f); glScalef(0.25f,0.22f,0.35f); glutSolidSphere(1.f,10,8); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.2f,0.4f,-0.6f); glScalef(0.3f,0.2f,0.28f);  glutSolidSphere(1.f,10,8); glPopMatrix();
}

static void drawLeg(float phase) {
    GLUquadric* q = gluNewQuadric();
    float sw = sinf(phase)*20.f;
    glPushMatrix();
    glRotatef(sw,1,0,0);
    glColor3f(0.88f,0.84f,0.78f);
    glPushMatrix(); glTranslatef(0,-0.4f,0); glScalef(0.22f,0.45f,0.22f); glutSolidSphere(1.f,8,6); glPopMatrix();
    glTranslatef(0,-0.8f,0);
    glRotatef(-fabsf(sw)*0.5f,1,0,0);
    glColor3f(0.82f,0.78f,0.72f);
    glPushMatrix(); glTranslatef(0,-0.3f,0); glScalef(0.16f,0.38f,0.16f); glutSolidSphere(1.f,8,6); glPopMatrix();
    glTranslatef(0,-0.62f,0);
    glColor3f(0.25f,0.2f,0.18f);
    glRotatef(-90.f,1,0,0);
    gluCylinder(q,0.1f,0.08f,0.15f,8,2);
    glPopMatrix();
    gluDeleteQuadric(q);
}

static void drawUdder() {
    float swing = isMillking ? sinf(udderSwing)*0.06f : 0.f;
    glColor3f(0.96f,0.78f,0.80f);
    glPushMatrix(); glTranslatef(0,swing,0); glScalef(0.45f,0.3f,0.4f); glutSolidSphere(1.f,12,8); glPopMatrix();
    GLUquadric* q = gluNewQuadric();
    glColor3f(0.9f,0.6f,0.65f);
    float px[4]={-0.15f,0.15f,-0.15f,0.15f};
    float pz[4]={-0.15f,-0.15f,0.15f,0.15f};
    for(int k=0;k<4;k++){
        glPushMatrix();
        glTranslatef(px[k],swing-0.25f,pz[k]);
        glRotatef(90.f,1,0,0);
        gluCylinder(q,0.04f,0.03f,0.15f,6,2);
        if(isMillking && k%2==0){
            glColor3f(1.f,1.f,1.f);
            glTranslatef(0,0,0.18f+sinf(milkAnim+k)*0.05f);
            glutSolidSphere(0.035f,6,4);
            glColor3f(0.9f,0.6f,0.65f);
        }
        glPopMatrix();
    }
    gluDeleteQuadric(q);
}

static void drawTail() {
    GLUquadric* q = gluNewQuadric();
    float tw = sinf(tailPhase)*25.f;
    glPushMatrix();
    glRotatef(tw,0,0,1);
    glRotatef(30.f,1,0,0);
    glColor3f(0.85f,0.8f,0.74f);
    glRotatef(-90.f,1,0,0);
    gluCylinder(q,0.07f,0.03f,0.8f,8,4);
    glTranslatef(0,0,0.8f);
    glColor3f(0.25f,0.2f,0.18f);
    glutSolidSphere(0.12f,8,6);
    glPopMatrix();
    gluDeleteQuadric(q);
}

// ---- Public ----

void drawTong() {
    glPushMatrix();
    glTranslatef(TONG_X, 0.f, TONG_Z);
    glColor3f(0.85f,0.15f,0.1f);
    GLUquadric* q = gluNewQuadric();
    glPushMatrix();
    glRotatef(-90,1,0,0);
    gluCylinder(q,0.5f,0.45f,1.4f,16,4);
    gluDisk(q,0,0.5f,16,1);
    glTranslatef(0,0,1.4f);
    gluDisk(q,0,0.45f,16,1);
    glPopMatrix();
    glColor3f(1.f,1.f,1.f);
    glLineWidth(2.f);
    glBegin(GL_LINE_LOOP);
    for(int k=0;k<16;k++){ float a=k*2*PI/16; glVertex3f(0.46f*cosf(a),0.7f,0.46f*sinf(a)); }
    glEnd();
    glLineWidth(1.f);
    glColor3f(0.7f,0.7f,0.75f);
    glPushMatrix(); glTranslatef(0,1.45f,0); glRotatef(-90,1,0,0); gluCylinder(q,0.3f,0.3f,0.08f,16,1); gluDisk(q,0,0.3f,16,1); glPopMatrix();
    gluDeleteQuadric(q);
    glPopMatrix();
}

void drawCow() {
    glPushMatrix();
    glTranslatef(cowX,0.f,cowZ);
    glRotatef(cowAngle,0,1,0);
    glTranslatef(0,1.3f,0);
    drawCowBody();
    glPushMatrix(); glTranslatef(0.f,0.35f,1.35f); glRotatef(-15.f,1,0,0); drawCowHead(); glPopMatrix();
    glColor3f(0.90f,0.86f,0.80f);
    glPushMatrix();
    glTranslatef(0.f,0.22f,1.0f); glRotatef(-75.f,1,0,0);
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q,0.28f,0.32f,0.5f,12,4);
    gluDeleteQuadric(q);
    glPopMatrix();
    glPushMatrix(); glTranslatef(-0.45f,-0.55f, 0.7f); drawLeg(walkPhase);      glPopMatrix();
    glPushMatrix(); glTranslatef( 0.45f,-0.55f, 0.7f); drawLeg(walkPhase+PI);   glPopMatrix();
    glPushMatrix(); glTranslatef(-0.45f,-0.55f,-0.7f); drawLeg(walkPhase+PI);   glPopMatrix();
    glPushMatrix(); glTranslatef( 0.45f,-0.55f,-0.7f); drawLeg(walkPhase);      glPopMatrix();
    glPushMatrix(); glTranslatef(0.f,-0.68f,-0.2f); drawUdder(); glPopMatrix();
    glPushMatrix(); glTranslatef(0.f,0.1f,-1.38f); drawTail(); glPopMatrix();
    glPopMatrix();
}

void updateCowAI() {
    if(isMillking) return;

    Waypoint &target = cowWaypoints[cowCurrentWaypoint];
    float dx = target.x - cowX;
    float dz = target.z - cowZ;
    float dist = sqrtf(dx*dx + dz*dz);

    if(dist < 0.15f) {
        cowCurrentWaypoint = (cowCurrentWaypoint + 1) % cowWaypointCount;
    } else {
        float targetAngle = atan2f(dx, dz) * 180.0f / PI;
        float diff = targetAngle - cowAngle;
        while(diff >  180.0f) diff -= 360.0f;
        while(diff < -180.0f) diff += 360.0f;
        cowAngle += diff * 0.05f;

        float moveRad = cowAngle * (PI / 180.0f);
        cowX += sinf(moveRad) * cowMoveSpeed;
        cowZ += cosf(moveRad) * cowMoveSpeed;
        walkPhase += 0.08f;
    }
}
