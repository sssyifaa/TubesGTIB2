#include "bro.h"
#include <GL/glut.h>

// Helper warna
static void setColor(float r, float g, float b) {
    glColor3f(r, g, b);
}

void updateBro(float dt)
{
    // Saat ini Bro NPC diam.
    // Parameter dt sengaja tidak dipakai.
    (void)dt;
}

void drawBro()
{
    glPushMatrix();
    glTranslatef(4.0f, 1.5f, -33.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

    // KAKI KIRI
    glPushMatrix();
    glTranslatef(-0.18f, -0.5f, 0.0f);
    setColor(0.77f, 0.66f, 0.51f);
    glPushMatrix();
    glTranslatef(0.0f,-0.55f,0.0f);
    glScalef(0.30f,0.85f,0.30f);
    glutSolidCube(1.0f);
    glPopMatrix();

    setColor(0.55f,0.39f,0.25f);
    glPushMatrix();
    glTranslatef(0.0f,-1.0f,-0.08f);
    glScalef(0.32f,0.14f,0.42f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    // KAKI KANAN
    glPushMatrix();
    glTranslatef(0.18f,-0.5f,0.0f);
    setColor(0.77f,0.66f,0.51f);
    glPushMatrix();
    glTranslatef(0.0f,-0.55f,0.0f);
    glScalef(0.30f,0.85f,0.30f);
    glutSolidCube(1.0f);
    glPopMatrix();

    setColor(0.55f,0.39f,0.25f);
    glPushMatrix();
    glTranslatef(0.0f,-1.0f,-0.08f);
    glScalef(0.32f,0.14f,0.42f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    // BADAN
    setColor(0.25f, 0.35f, 0.55f);
    glPushMatrix();
    glScalef(0.75f,0.95f,0.38f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // RANSEL
    setColor(0.40f, 0.55f, 0.35f);
    glPushMatrix();
    glTranslatef(0.0f,0.08f,0.22f);
    glScalef(0.55f,0.65f,0.18f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // TANGAN KIRI
    glPushMatrix();
    glTranslatef(-0.48f,0.30f,0.0f);

    setColor(0.25f,0.35f,0.55f);
    glPushMatrix();
    glTranslatef(0.0f,-0.30f,0.0f);
    glScalef(0.22f,0.60f,0.22f);
    glutSolidCube(1.0f);
    glPopMatrix();

    setColor(0.88f,0.70f,0.52f);
    glPushMatrix();
    glTranslatef(0.0f,-0.68f,0.0f);
    glutSolidSphere(0.10f,12,12);
    glPopMatrix();
    glPopMatrix();

    // TANGAN KANAN
    glPushMatrix();
    glTranslatef(0.48f,0.30f,0.0f);

    setColor(0.25f,0.35f,0.55f);
    glPushMatrix();
    glTranslatef(0.0f,-0.30f,0.0f);
    glScalef(0.22f,0.60f,0.22f);
    glutSolidCube(1.0f);
    glPopMatrix();

    setColor(0.88f,0.70f,0.52f);
    glPushMatrix();
    glTranslatef(0.0f,-0.68f,0.0f);
    glutSolidSphere(0.10f,12,12);
    glPopMatrix();
    glPopMatrix();

    // LEHER
    setColor(0.88f,0.70f,0.52f);
    glPushMatrix();
    glTranslatef(0.0f,0.53f,0.0f);
    glScalef(0.20f,0.18f,0.20f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // KEPALA
    setColor(0.88f,0.70f,0.52f);
    glPushMatrix();
    glTranslatef(0.0f,0.78f,0.0f);
    glScalef(0.58f,0.58f,0.54f);
    glutSolidCube(1.0f);

    setColor(0.17f,0.10f,0.05f);
    glPushMatrix();
    glTranslatef(-0.22f,0.05f,-0.52f);
    glutSolidSphere(0.07f,10,10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.22f,0.05f,-0.52f);
    glutSolidSphere(0.07f,10,10);
    glPopMatrix();

    setColor(0.75f,0.56f,0.38f);
    glPushMatrix();
    glTranslatef(0.0f,-0.04f,-0.54f);
    glutSolidSphere(0.045f,8,8);
    glPopMatrix();

    glPopMatrix();

    // RAMBUT
    setColor(0.15f,0.09f,0.04f);

    glPushMatrix();
    glTranslatef(0.0f,1.12f,-0.02f);
    glScalef(0.56f,0.10f,0.52f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.33f,0.92f,0.0f);
    glScalef(0.10f,0.46f,0.50f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.33f,0.92f,0.0f);
    glScalef(0.10f,0.46f,0.50f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f,0.85f,0.30f);
    glScalef(0.54f,0.58f,0.10f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}
