#include "player.h"
#include "global.h"
#include "collision.h"
#include "utils.h"
#include <GL/glut.h>
#include <cmath>

static const float PI         = 3.14159265f;
static const float GRAVITY    = 0.012f;
static const float JUMP_FORCE = 0.22f;
static const float GROUND_Y   = 0.0f;
static const float TURN_SPEED = 3.0f;

void updatePlayer() {
    float rad = playerYaw * (PI / 180.0f);
    bool isWalking = false;

    bool moveForward  = keys['w'] || keys['W'] || specialKeys[GLUT_KEY_UP];
    bool moveBackward = keys['s'] || keys['S'] || specialKeys[GLUT_KEY_DOWN];
    bool turnLeft     = keys['a'] || keys['A'] || specialKeys[GLUT_KEY_LEFT];
    bool turnRight    = keys['d'] || keys['D'] || specialKeys[GLUT_KEY_RIGHT];

    if(moveForward) {
        float nx = playerX - sinf(rad) * moveSpeed;
        float nz = playerZ - cosf(rad) * moveSpeed;
        if(!checkCollision(nx, playerZ)) playerX = nx;
        if(!checkCollision(playerX, nz)) playerZ = nz;
        isWalking = true;
    }
    if(moveBackward) {
        float nx = playerX + sinf(rad) * moveSpeed;
        float nz = playerZ + cosf(rad) * moveSpeed;
        if(!checkCollision(nx, playerZ)) playerX = nx;
        if(!checkCollision(playerX, nz)) playerZ = nz;
        isWalking = true;
    }
    if(turnLeft)  playerYaw += TURN_SPEED;
    if(turnRight) playerYaw -= TURN_SPEED;

    if(keys[' '] && isGrounded) { velocityY = JUMP_FORCE; isGrounded = false; }
    if(!isGrounded) velocityY -= GRAVITY;
    playerY += velocityY;
    if(playerY <= GROUND_Y) { playerY = GROUND_Y; velocityY = 0.0f; isGrounded = true; }

    if(isGrounded) {
        if(isWalking) { walkAnim += 0.25f; swingAngle = sinf(walkAnim) * 42.0f; }
        else {
            if(swingAngle >  0) swingAngle -= 2.0f;
            if(swingAngle <  0) swingAngle += 2.0f;
            if(fabsf(swingAngle) < 2.0f) swingAngle = 0.0f;
        }
    } else { swingAngle = 0.0f; }
}

void drawPlayer() {
    glPushMatrix();
    glTranslatef(playerX, playerY + 1.5f, playerZ);
    glRotatef(playerYaw, 0.f, 1.f, 0.f);

    float armJumpPose = isGrounded ? 0.0f : -35.0f;

    // Kaki Kiri
    glPushMatrix(); glTranslatef(-0.18f,-0.5f,0.0f); glRotatef(-swingAngle,1.f,0.f,0.f);
    setColor(0.77f,0.66f,0.51f);
    glPushMatrix(); glTranslatef(0,-0.55f,0); glScalef(0.30f,0.85f,0.30f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.55f,0.39f,0.25f);
    glPushMatrix(); glTranslatef(0,-1.0f,-0.08f); glScalef(0.32f,0.14f,0.42f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    // Kaki Kanan
    glPushMatrix(); glTranslatef(0.18f,-0.5f,0.0f); glRotatef(swingAngle,1.f,0.f,0.f);
    setColor(0.77f,0.66f,0.51f);
    glPushMatrix(); glTranslatef(0,-0.55f,0); glScalef(0.30f,0.85f,0.30f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.55f,0.39f,0.25f);
    glPushMatrix(); glTranslatef(0,-1.0f,-0.08f); glScalef(0.32f,0.14f,0.42f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    // Badan
    setColor(0.18f,0.23f,0.29f);
    glPushMatrix(); glScalef(0.75f,0.95f,0.38f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.29f,0.39f,0.50f);
    glPushMatrix(); glTranslatef(0,0.3f,-0.20f); glScalef(0.28f,0.28f,0.04f); glutSolidCube(1.0f); glPopMatrix();

    // Tas
    setColor(0.77f,0.42f,0.29f);
    glPushMatrix(); glTranslatef(0,0.08f,0.22f); glScalef(0.55f,0.65f,0.18f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.85f,0.52f,0.37f);
    glPushMatrix(); glTranslatef(0,-0.08f,0.32f); glScalef(0.35f,0.32f,0.06f); glutSolidCube(1.0f); glPopMatrix();

    // Tangan Kiri
    glPushMatrix(); glTranslatef(-0.48f,0.30f,0.0f); glRotatef(swingAngle+armJumpPose,1.f,0.f,0.f);
    setColor(0.18f,0.23f,0.29f);
    glPushMatrix(); glTranslatef(0,-0.30f,0); glScalef(0.22f,0.60f,0.22f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.76f,0.57f,0.37f);
    glPushMatrix(); glTranslatef(0,-0.68f,0); glutSolidSphere(0.10f,12,12); glPopMatrix();
    glPopMatrix();

    // Tangan Kanan
    glPushMatrix(); glTranslatef(0.48f,0.30f,0.0f); glRotatef(-swingAngle+armJumpPose,1.f,0.f,0.f);
    setColor(0.18f,0.23f,0.29f);
    glPushMatrix(); glTranslatef(0,-0.30f,0); glScalef(0.22f,0.60f,0.22f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.76f,0.57f,0.37f);
    glPushMatrix(); glTranslatef(0,-0.68f,0); glutSolidSphere(0.10f,12,12); glPopMatrix();
    glPopMatrix();

    // Leher
    setColor(0.76f,0.57f,0.37f);
    glPushMatrix(); glTranslatef(0,0.53f,0); glScalef(0.20f,0.18f,0.20f); glutSolidCube(1.0f); glPopMatrix();

    // Kepala
    setColor(0.76f,0.57f,0.37f);
    glPushMatrix();
    glTranslatef(0,0.78f,0); glScalef(0.58f,0.58f,0.54f); glutSolidCube(1.0f);
    setColor(0.17f,0.10f,0.05f);
    glPushMatrix(); glTranslatef(-0.22f,0.05f,-0.52f); glutSolidSphere(0.07f,10,10); glPopMatrix();
    setColor(0.95f,0.95f,0.95f);
    glPushMatrix(); glTranslatef(-0.18f,0.08f,-0.54f); glutSolidSphere(0.025f,8,8); glPopMatrix();
    setColor(0.17f,0.10f,0.05f);
    glPushMatrix(); glTranslatef(0.22f,0.05f,-0.52f); glutSolidSphere(0.07f,10,10); glPopMatrix();
    setColor(0.95f,0.95f,0.95f);
    glPushMatrix(); glTranslatef(0.26f,0.08f,-0.54f); glutSolidSphere(0.025f,8,8); glPopMatrix();
    setColor(0.65f,0.46f,0.28f);
    glPushMatrix(); glTranslatef(0,-0.04f,-0.54f); glutSolidSphere(0.045f,8,8); glPopMatrix();
    glPopMatrix();

    // Rambut
    setColor(0.42f,0.31f,0.23f);
    glPushMatrix(); glTranslatef(-0.36f,0.98f, 0.12f); glutSolidSphere(0.150f,12,12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.38f,0.92f,-0.05f); glutSolidSphere(0.160f,12,12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.34f,0.88f,-0.22f); glutSolidSphere(0.145f,12,12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.36f,0.98f, 0.12f); glutSolidSphere(0.150f,12,12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.38f,0.92f,-0.05f); glutSolidSphere(0.160f,12,12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.34f,0.88f,-0.22f); glutSolidSphere(0.145f,12,12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.18f,0.98f, 0.35f); glutSolidSphere(0.150f,12,12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.00f,0.96f, 0.38f); glutSolidSphere(0.165f,12,12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.18f,0.98f, 0.35f); glutSolidSphere(0.150f,12,12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.12f,0.87f, 0.36f); glutSolidSphere(0.140f,12,12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.12f,0.87f, 0.36f); glutSolidSphere(0.140f,12,12); glPopMatrix();

    // Topi
    setColor(0.40f,0.30f,0.22f);
    glPushMatrix(); glTranslatef(0,0.75f,0.30f); glScalef(0.44f,0.48f,0.08f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0,0.56f,0.29f); glScalef(0.34f,0.20f,0.07f); glutSolidCube(1.0f); glPopMatrix();

    setColor(0.15f,0.19f,0.25f);
    glPushMatrix();
    glTranslatef(0,0.78f,0);
    glPushMatrix(); glTranslatef(0,0.28f,-0.04f); glScalef(0.66f,0.10f,0.60f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.77f,0.42f,0.29f);
    glPushMatrix(); glTranslatef(0,0.46f,0.02f); glScalef(0.52f,0.28f,0.50f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    glPopMatrix();
}
