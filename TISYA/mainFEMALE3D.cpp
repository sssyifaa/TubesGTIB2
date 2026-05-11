#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

// --- (posisi dikunci) ---
float playerX = 0.0f, playerY = 1.0f, playerZ = 0.0f;
float playerYaw = 180.0f; 

// --- Animasi Idle ---
float idleTime = 0.0f;   

// --- Sistem Kamera Orbit ---
float camYaw = 0.0f;
float camPitch = 15.0f;
float camDistance = 7.0f; 
int lastMouseX = -1, lastMouseY = -1;
bool isDragging = false;

#define PI 3.14159265f
void setColor(float r, float g, float b) {
    glColor3f(r, g, b);
}

void init()
{
    glClearColor(0.83f, 0.79f, 0.77f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    float lightPos[] = {8.0f, 15.0f, 6.0f, 1.0f};
    float ambient[]  = {0.48f, 0.44f, 0.42f, 1.0f};
    float diffuse[]  = {0.92f, 0.88f, 0.84f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    float lightPos2[] = {-5.0f, 3.0f, -8.0f, 1.0f};
    float ambient2[]  = {0.22f, 0.20f, 0.22f, 1.0f};
    float diffuse2[]  = {0.28f, 0.26f, 0.30f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos2);
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient2);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse2);
}

void drawFloor()
{
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    for (int i = -20; i < 20; i++) {
        for (int j = -20; j < 20; j++) {
            if ((i + j) % 2 == 0)
                glColor3f(0.79f, 0.69f, 0.66f); 
            else
                glColor3f(0.69f, 0.58f, 0.56f); 

            glVertex3f(i,     0.0f, j);
            glVertex3f(i,     0.0f, j + 1);
            glVertex3f(i + 1, 0.0f, j + 1);
            glVertex3f(i + 1, 0.0f, j);
        }
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawPlayer()
{
    float armSwing = sin(idleTime) * 18.0f; 

    glPushMatrix();
    glTranslatef(playerX, playerY, playerZ);
    glRotatef(playerYaw, 0.0f, 1.0f, 0.0f);

    // ---- 1. KAKI KIRI ----
    setColor(0.91f, 0.87f, 0.82f);
    glPushMatrix(); glTranslatef(-0.14f, 0.1f, 0.0f);
    glPushMatrix(); glTranslatef(0.0f, -0.55f, 0.0f); glScalef(0.28f, 0.85f, 0.28f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.48f, 0.29f, 0.31f);
    glPushMatrix(); glTranslatef(0.0f, -1.0f, -0.06f); glScalef(0.28f, 0.13f, 0.38f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    // ---- 2. KAKI KANAN ----
    setColor(0.91f, 0.87f, 0.82f);
    glPushMatrix(); glTranslatef(0.14f, 0.1f, 0.0f);
    glPushMatrix(); glTranslatef(0.0f, -0.55f, 0.0f); glScalef(0.28f, 0.85f, 0.28f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.48f, 0.29f, 0.31f);
    glPushMatrix(); glTranslatef(0.0f, -1.0f, -0.06f); glScalef(0.28f, 0.13f, 0.38f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    // ---- 3. ROK ----
    setColor(0.91f, 0.87f, 0.82f);
    glPushMatrix(); glTranslatef(0.0f, -0.10f, 0.0f); glScalef(0.78f, 0.35f, 0.40f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.91f, 0.87f, 0.82f);
    glPushMatrix(); glTranslatef(0.0f, -0.33f, 0.0f); glScalef(0.90f, 0.22f, 0.46f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.69f, 0.44f, 0.38f);
    glPushMatrix(); glTranslatef(0.0f, -0.44f, 0.0f); glScalef(0.91f, 0.04f, 0.47f); glutSolidCube(1.0f); glPopMatrix();

    // ---- 4. BADAN (blouse) ----
    setColor(0.83f, 0.56f, 0.56f);
    glPushMatrix(); glScalef(0.70f, 0.90f, 0.36f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.95f, 0.92f, 0.88f);
    glPushMatrix(); glTranslatef( 0.10f, 0.32f, -0.19f); glScalef(0.14f, 0.22f, 0.04f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.10f, 0.32f, -0.19f); glScalef(0.14f, 0.22f, 0.04f); glutSolidCube(1.0f); glPopMatrix();

    // ---- 5. TANGAN KIRI ----
    glPushMatrix();
    glTranslatef(-0.44f, 0.28f, 0.0f);
    glRotatef(armSwing, 1.0f, 0.0f, 0.0f);
    setColor(0.83f, 0.56f, 0.56f);
    glPushMatrix(); glTranslatef(0.0f, -0.28f, 0.0f); glScalef(0.20f, 0.56f, 0.20f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.76f, 0.57f, 0.38f);
    glPushMatrix(); glTranslatef(0.0f, -0.64f, 0.0f); glutSolidSphere(0.095f, 12, 12); glPopMatrix();
    glPopMatrix();

    // ---- 6. TANGAN KANAN ----
    glPushMatrix();
    glTranslatef(0.44f, 0.28f, 0.0f);
    glRotatef(-armSwing, 1.0f, 0.0f, 0.0f);
    setColor(0.83f, 0.56f, 0.56f);
    glPushMatrix(); glTranslatef(0.0f, -0.28f, 0.0f); glScalef(0.20f, 0.56f, 0.20f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.76f, 0.57f, 0.38f);
    glPushMatrix(); glTranslatef(0.0f, -0.64f, 0.0f); glutSolidSphere(0.095f, 12, 12); glPopMatrix();
    glPopMatrix();

    // ---- 7. LEHER ----
    setColor(0.76f, 0.57f, 0.38f);
    glPushMatrix(); glTranslatef(0.0f, 0.52f, 0.0f); glScalef(0.18f, 0.16f, 0.18f); glutSolidCube(1.0f); glPopMatrix();

    // ---- 8. KEPALA ----
    setColor(0.76f, 0.57f, 0.38f);
    glPushMatrix();
    glTranslatef(0.0f, 0.78f, 0.0f);
    glScalef(0.56f, 0.56f, 0.52f);
    glutSolidCube(1.0f);

    setColor(0.18f, 0.12f, 0.08f);
    glPushMatrix(); glTranslatef(-0.21f, 0.05f, -0.52f); glutSolidSphere(0.068f, 10, 10); glPopMatrix();
    setColor(0.95f, 0.95f, 0.95f);
    glPushMatrix(); glTranslatef(-0.17f, 0.08f, -0.54f); glutSolidSphere(0.022f, 8, 8); glPopMatrix();

    setColor(0.18f, 0.12f, 0.08f);
    glPushMatrix(); glTranslatef(0.21f, 0.05f, -0.52f); glutSolidSphere(0.068f, 10, 10); glPopMatrix();
    setColor(0.95f, 0.95f, 0.95f);
    glPushMatrix(); glTranslatef(0.25f, 0.08f, -0.54f); glutSolidSphere(0.022f, 8, 8); glPopMatrix();

    setColor(0.64f, 0.45f, 0.28f);
    glPushMatrix(); glTranslatef(0.0f, -0.04f, -0.53f); glutSolidSphere(0.040f, 8, 8); glPopMatrix();
    setColor(0.78f, 0.45f, 0.45f);
    glPushMatrix(); glTranslatef(0.0f, -0.15f, -0.53f); glScalef(0.18f, 0.06f, 0.05f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix(); 

    // ---- 9. RAMBUT BONDOL ----
    setColor(0.16f, 0.15f, 0.15f);
    glPushMatrix(); glTranslatef(0.0f, 1.06f, 0.02f); glScalef(0.60f, 0.18f, 0.56f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.32f, 0.84f, 0.00f); glScalef(0.10f, 0.44f, 0.50f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.32f, 0.84f, 0.00f); glScalef(0.10f, 0.44f, 0.50f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.84f, 0.30f); glScalef(0.56f, 0.46f, 0.10f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.99f, -0.24f); glScalef(0.52f, 0.12f, 0.10f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.93f, -0.27f); glScalef(0.32f, 0.10f, 0.06f); glPopMatrix();

    glPopMatrix(); // end player
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float radYaw = camYaw * (PI / 180.0f);
    float radPitch = camPitch * (PI / 180.0f);
    
    float targetX = playerX;
    float targetY = playerY + 0.5f; 
    float targetZ = playerZ;

    float eyeX = targetX + camDistance * cos(radPitch) * sin(radYaw);
    float eyeY = targetY + camDistance * sin(radPitch);
    float eyeZ = targetZ + camDistance * cos(radPitch) * cos(radYaw);

    gluLookAt(
        eyeX, eyeY, eyeZ,  
        targetX, targetY, targetZ,
        0.0f, 1.0f, 0.0f 
    );

    drawFloor();
    drawPlayer();

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    if (h == 0) h = 1;
    float ratio = (float)w / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, ratio, 0.1, 100);
    glViewport(0, 0, w, h);
}

// --- FUNGSI KLIK MOUSE ---
void mouseClick(int button, int state, int x, int y) 
{
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            isDragging = false;
        }
    }
    
    // Zoom In / Zoom Out menggunakan Scroll Wheel Mouse
    if (state == GLUT_DOWN) {
        if (button == 3) camDistance -= 0.5f; // Scroll Up
        if (button == 4) camDistance += 0.5f; // Scroll Down
    }
    
    // Batasi Zoom agar tidak tembus ke dalam karakter
    if (camDistance < 2.5f) camDistance = 2.5f;
    if (camDistance > 15.0f) camDistance = 15.0f;
}

// --- FUNGSI GESER MOUSE ---
void mouseMotion(int x, int y) 
{
    if (isDragging) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        camYaw += dx * 0.5f;
        camPitch += dy * 0.5f; 

        if (camPitch > 89.0f) camPitch = 89.0f;
        if (camPitch < -89.0f) camPitch = -89.0f;

        lastMouseX = x;
        lastMouseY = y;
    }
}

void keyboardDown(unsigned char key, int x, int y) {
    if (key == 27) exit(0); 
}

void update(int value)
{
    idleTime += 0.035f; 
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Female 3D Viewer Mode");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMotion);

    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}
