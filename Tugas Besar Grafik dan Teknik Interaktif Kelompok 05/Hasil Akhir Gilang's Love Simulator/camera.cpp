#include "camera.h"
#include "global.h"
#include <GL/glut.h>
#include <cmath>

static const float PI  = 3.14159265f;
static const float D2R = PI / 180.0f;

static float camDistance  = 6.0f;
static float camPitch     = 20.0f;
static float camYawOffset = 0.0f;

void updateCamera() {
    glLoadIdentity();

    float px = playerX, py = playerY + 2.0f, pz = playerZ;

    // Jarak kamera: pendek di dalam rumah supaya tidak tembus dinding
    float dist  = insideRumahBro ? 3.5f  : camDistance;
    float pitch = insideRumahBro ? 25.0f : camPitch;

    float hDist = dist * cosf(pitch * D2R);
    float vDist = dist * sinf(pitch * D2R);
    float theta = (playerYaw + camYawOffset) * D2R;

    float cx = px + hDist * sinf(theta);
    float cz = pz + hDist * cosf(theta);
    float cy = py + vDist;

    gluLookAt(cx, cy, cz,
              px, py, pz,
              0.0f, 1.0f, 0.0f);
}

void handleCamLook(int x, int y, int screenW, int screenH) {
    int cx = screenW / 2;
    int cy = screenH / 2;

    static bool firstMouse = true;
    if (firstMouse) {
        firstMouse = false;
        glutWarpPointer(cx, cy);
        return;
    }

    if (x == cx && y == cy) return;

    float dx = (float)(x - cx);
    float dy = (float)(y - cy);
    float sensitivity = 0.15f;

    playerYaw += dx * sensitivity;
    camPitch  -= dy * sensitivity;

    if (camPitch >  60.0f) camPitch =  60.0f;
    if (camPitch < -10.0f) camPitch = -10.0f;

    glutWarpPointer(cx, cy);
}
