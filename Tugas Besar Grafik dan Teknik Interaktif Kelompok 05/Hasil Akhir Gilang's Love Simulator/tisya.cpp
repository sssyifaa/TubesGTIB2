#include "tisya.h"
#include "global.h"
#include <GL/glut.h>
#include <cmath>

void updateTisya(float dt) {
    tisyaIdleTime += dt;
}

void drawTisya() {
    float armSwing = sinf(tisyaIdleTime) * 18.0f;

    glPushMatrix();
    glTranslatef(tisyaX, 1.0f, tisyaZ);
    glRotatef(tisyaYaw, 0.0f, 1.0f, 0.0f);

    // Kaki Kiri
    glColor3f(0.91f, 0.87f, 0.82f);
    glPushMatrix(); glTranslatef(-0.14f, 0.1f, 0.0f);
    glPushMatrix(); glTranslatef(0.0f,-0.55f,0.0f); glScalef(0.28f,0.85f,0.28f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.48f,0.29f,0.31f);
    glPushMatrix(); glTranslatef(0.0f,-1.0f,-0.06f); glScalef(0.28f,0.13f,0.38f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    // Kaki Kanan
    glColor3f(0.91f,0.87f,0.82f);
    glPushMatrix(); glTranslatef(0.14f,0.1f,0.0f);
    glPushMatrix(); glTranslatef(0.0f,-0.55f,0.0f); glScalef(0.28f,0.85f,0.28f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.48f,0.29f,0.31f);
    glPushMatrix(); glTranslatef(0.0f,-1.0f,-0.06f); glScalef(0.28f,0.13f,0.38f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    // Rok
    glColor3f(0.91f,0.87f,0.82f);
    glPushMatrix(); glTranslatef(0.0f,-0.10f,0.0f); glScalef(0.78f,0.35f,0.40f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f,-0.33f,0.0f); glScalef(0.90f,0.22f,0.46f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.69f,0.44f,0.38f);
    glPushMatrix(); glTranslatef(0.0f,-0.44f,0.0f); glScalef(0.91f,0.04f,0.47f); glutSolidCube(1.0f); glPopMatrix();

    // Badan
    glColor3f(0.52f,0.72f,0.88f);
    glPushMatrix(); glScalef(0.70f,0.90f,0.36f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.95f,0.92f,0.88f);
    glPushMatrix(); glTranslatef( 0.10f,0.32f,-0.19f); glScalef(0.14f,0.22f,0.04f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.10f,0.32f,-0.19f); glScalef(0.14f,0.22f,0.04f); glutSolidCube(1.0f); glPopMatrix();

    // Tangan Kiri
    glPushMatrix();
    glTranslatef(-0.44f,0.28f,0.0f);
    glRotatef(armSwing,1.0f,0.0f,0.0f);
    glColor3f(0.52f,0.72f,0.88f);
    glPushMatrix(); glTranslatef(0.0f,-0.28f,0.0f); glScalef(0.20f,0.56f,0.20f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.76f,0.57f,0.38f);
    glPushMatrix(); glTranslatef(0.0f,-0.64f,0.0f); glutSolidSphere(0.095f,12,12); glPopMatrix();
    glPopMatrix();

    // Tangan Kanan
    glPushMatrix();
    glTranslatef(0.44f,0.28f,0.0f);
    glRotatef(-armSwing,1.0f,0.0f,0.0f);
    glColor3f(0.52f,0.72f,0.88f);
    glPushMatrix(); glTranslatef(0.0f,-0.28f,0.0f); glScalef(0.20f,0.56f,0.20f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.76f,0.57f,0.38f);
    glPushMatrix(); glTranslatef(0.0f,-0.64f,0.0f); glutSolidSphere(0.095f,12,12); glPopMatrix();
    glPopMatrix();

    // Leher
    glColor3f(0.76f,0.57f,0.38f);
    glPushMatrix(); glTranslatef(0.0f,0.52f,0.0f); glScalef(0.18f,0.16f,0.18f); glutSolidCube(1.0f); glPopMatrix();

    // Kepala
    glColor3f(0.76f,0.57f,0.38f);
    glPushMatrix();
    glTranslatef(0.0f,0.78f,0.0f);
    glScalef(0.56f,0.56f,0.52f);
    glutSolidCube(1.0f);
    glColor3f(0.18f,0.12f,0.08f);
    glPushMatrix(); glTranslatef(-0.21f,0.05f,-0.52f); glutSolidSphere(0.068f,10,10); glPopMatrix();
    glColor3f(0.95f,0.95f,0.95f);
    glPushMatrix(); glTranslatef(-0.17f,0.08f,-0.54f); glutSolidSphere(0.022f,8,8); glPopMatrix();
    glColor3f(0.18f,0.12f,0.08f);
    glPushMatrix(); glTranslatef(0.21f,0.05f,-0.52f); glutSolidSphere(0.068f,10,10); glPopMatrix();
    glColor3f(0.95f,0.95f,0.95f);
    glPushMatrix(); glTranslatef(0.25f,0.08f,-0.54f); glutSolidSphere(0.022f,8,8); glPopMatrix();
    glColor3f(0.64f,0.45f,0.28f);
    glPushMatrix(); glTranslatef(0.0f,-0.04f,-0.53f); glutSolidSphere(0.040f,8,8); glPopMatrix();
    glColor3f(0.78f,0.45f,0.45f);
    glPushMatrix(); glTranslatef(0.0f,-0.15f,-0.53f); glScalef(0.18f,0.06f,0.05f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    // Rambut bondol
    glColor3f(0.16f,0.15f,0.15f);
    glPushMatrix(); glTranslatef(0.0f,1.06f,0.02f); glScalef(0.60f,0.18f,0.56f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.32f,0.84f,0.00f); glScalef(0.10f,0.44f,0.50f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.32f,0.84f,0.00f); glScalef(0.10f,0.44f,0.50f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f,0.84f,0.30f); glScalef(0.56f,0.46f,0.10f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f,0.99f,-0.24f); glScalef(0.52f,0.12f,0.10f); glutSolidCube(1.0f); glPopMatrix();

    glPopMatrix(); // end tisya
}
