#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>


float playerX = 0.0f, playerY = 1.0f, playerZ = 0.0f;
float playerYaw = 0.0f;
float moveSpeed = 0.08f; 
float turnSpeed = 3.0f;

float velocityY = 0.0f;
float gravity = 0.012f;
float jumpForce = 0.22f;
float groundY = 1.0f;
bool isGrounded = true;

bool keys[256] = {false};

float walkAnim = 0.0f;   
float swingAngle = 0.0f; 

#define PI 3.14159265f

void setColor(float r, float g, float b) {
    glColor3f(r, g, b);
}

void init()
{
    glClearColor(0.72f, 0.77f, 0.83f, 1.0f); 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    
    float lightPos[] = {8.0f, 15.0f, 6.0f, 1.0f};
    float ambient[]  = {0.45f, 0.42f, 0.40f, 1.0f};
    float diffuse[]  = {0.90f, 0.88f, 0.82f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    float lightPos2[] = {-5.0f, 3.0f, -8.0f, 1.0f};
    float ambient2[]  = {0.20f, 0.22f, 0.25f, 1.0f};
    float diffuse2[]  = {0.25f, 0.28f, 0.32f, 1.0f};
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
                glColor3f(0.49f, 0.62f, 0.48f);
            else
                glColor3f(0.35f, 0.48f, 0.34f);
            
            glVertex3f(i,     0.0f, j);
            glVertex3f(i,     0.0f, j + 1);
            glVertex3f(i + 1, 0.0f, j + 1);
            glVertex3f(i + 1, 0.0f, j);
        }
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawCylinder(float radius, float height, int slices) {
    float step = 2.0f * PI / slices;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++) {
        float angle = i * step;
        float nx = cos(angle);
        float nz = sin(angle);
        glNormal3f(nx, 0.0f, nz);
        glVertex3f(radius * nx, 0.0f,   radius * nz);
        glVertex3f(radius * nx, height, radius * nz);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, height, 0.0f);
    for (int i = 0; i <= slices; i++) {
        float angle = i * step;
        glVertex3f(radius * cos(angle), height, radius * sin(angle));
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = slices; i >= 0; i--) {
        float angle = i * step;
        glVertex3f(radius * cos(angle), 0.0f, radius * sin(angle));
    }
    glEnd();
}

void drawPlayer()
{
    glPushMatrix();
    glTranslatef(playerX, playerY, playerZ);
    glRotatef(playerYaw, 0.0f, 1.0f, 0.0f);

    float armJumpPose = isGrounded ? 0.0f : -35.0f;

    // ---- 1. KAKI KIRI ----
    glPushMatrix();
    glTranslatef(-0.18f, -0.5f, 0.0f);
    glRotatef(-swingAngle, 1.0f, 0.0f, 0.0f);
    // Paha + betis
    setColor(0.77f, 0.66f, 0.51f);
    glPushMatrix();
    glTranslatef(0.0f, -0.55f, 0.0f);
    glScalef(0.30f, 0.85f, 0.30f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Sepatu
    setColor(0.55f, 0.39f, 0.25f);
    glPushMatrix();
    glTranslatef(0.0f, -1.0f, -0.08f);
    glScalef(0.32f, 0.14f, 0.42f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    // ---- 2. KAKI KANAN ----
    glPushMatrix();
    glTranslatef(0.18f, -0.5f, 0.0f);
    glRotatef(swingAngle, 1.0f, 0.0f, 0.0f);
    setColor(0.77f, 0.66f, 0.51f);
    glPushMatrix();
    glTranslatef(0.0f, -0.55f, 0.0f);
    glScalef(0.30f, 0.85f, 0.30f);
    glutSolidCube(1.0f);
    glPopMatrix();
    setColor(0.55f, 0.39f, 0.25f);
    glPushMatrix();
    glTranslatef(0.0f, -1.0f, -0.08f);
    glScalef(0.32f, 0.14f, 0.42f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    // ---- 3. BADAN ----
    setColor(0.18f, 0.23f, 0.29f);
    glPushMatrix();
    glScalef(0.75f, 0.95f, 0.38f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Detail jacket
    setColor(0.29f, 0.39f, 0.50f);
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, -0.20f);
    glScalef(0.28f, 0.28f, 0.04f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ---- 4. RANSEL ----
    setColor(0.77f, 0.42f, 0.29f);
    glPushMatrix();
    glTranslatef(0.0f, 0.08f, 0.22f);
    glScalef(0.55f, 0.65f, 0.18f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Kantong kecil ransel
    setColor(0.85f, 0.52f, 0.37f);
    glPushMatrix();
    glTranslatef(0.0f, -0.08f, 0.32f);
    glScalef(0.35f, 0.32f, 0.06f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ---- 5. TANGAN KIRI ----
    glPushMatrix();
    glTranslatef(-0.48f, 0.30f, 0.0f);
    glRotatef(swingAngle + armJumpPose, 1.0f, 0.0f, 0.0f);
    // Lengan atas (jacket)
    setColor(0.18f, 0.23f, 0.29f);
    glPushMatrix();
    glTranslatef(0.0f, -0.30f, 0.0f);
    glScalef(0.22f, 0.60f, 0.22f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Tangan / kulit
    setColor(0.76f, 0.57f, 0.37f);
    glPushMatrix();
    glTranslatef(0.0f, -0.68f, 0.0f);
    glutSolidSphere(0.10f, 12, 12);
    glPopMatrix();
    glPopMatrix();

    // ---- 6. TANGAN KANAN ----
    glPushMatrix();
    glTranslatef(0.48f, 0.30f, 0.0f);
    glRotatef(-swingAngle + armJumpPose, 1.0f, 0.0f, 0.0f);
    setColor(0.18f, 0.23f, 0.29f);
    glPushMatrix();
    glTranslatef(0.0f, -0.30f, 0.0f);
    glScalef(0.22f, 0.60f, 0.22f);
    glutSolidCube(1.0f);
    glPopMatrix();
    setColor(0.76f, 0.57f, 0.37f);
    glPushMatrix();
    glTranslatef(0.0f, -0.68f, 0.0f);
    glutSolidSphere(0.10f, 12, 12);
    glPopMatrix();
    glPopMatrix();

    // ---- 7. LEHER (skin) ----
    setColor(0.76f, 0.57f, 0.37f);
    glPushMatrix();
    glTranslatef(0.0f, 0.53f, 0.0f);
    glScalef(0.20f, 0.18f, 0.20f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ---- 8. KEPALA ----
    setColor(0.76f, 0.57f, 0.37f);
    glPushMatrix();
    glTranslatef(0.0f, 0.78f, 0.0f);
    glScalef(0.58f, 0.58f, 0.54f);
    glutSolidCube(1.0f);

    // Mata kiri
    setColor(0.17f, 0.10f, 0.05f);
    glPushMatrix();
    glTranslatef(-0.22f, 0.05f, -0.52f);
    glutSolidSphere(0.07f, 10, 10);
    glPopMatrix();
    // Highlight mata kiri
    setColor(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(-0.18f, 0.08f, -0.54f);
    glutSolidSphere(0.025f, 8, 8);
    glPopMatrix();

    // Mata kanan
    setColor(0.17f, 0.10f, 0.05f);
    glPushMatrix();
    glTranslatef(0.22f, 0.05f, -0.52f);
    glutSolidSphere(0.07f, 10, 10);
    glPopMatrix();
    setColor(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.26f, 0.08f, -0.54f);
    glutSolidSphere(0.025f, 8, 8);
    glPopMatrix();

    // Hidung kecil
    setColor(0.65f, 0.46f, 0.28f);
    glPushMatrix();
    glTranslatef(0.0f, -0.04f, -0.54f);
    glutSolidSphere(0.045f, 8, 8);
    glPopMatrix();

    glPopMatrix(); // end kepala

    // ---- 8b. RAMBUT IKAL ----
    setColor(0.42f, 0.31f, 0.23f);

    // --- Sisi KIRI ---
    glPushMatrix(); glTranslatef(-0.36f, 0.98f,  0.12f); glutSolidSphere(0.150f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.38f, 0.92f, -0.05f); glutSolidSphere(0.160f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.34f, 0.88f, -0.22f); glutSolidSphere(0.145f, 12, 12); glPopMatrix();

    // --- Sisi KANAN ---
    glPushMatrix(); glTranslatef( 0.36f, 0.98f,  0.12f); glutSolidSphere(0.150f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.38f, 0.92f, -0.05f); glutSolidSphere(0.160f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.34f, 0.88f, -0.22f); glutSolidSphere(0.145f, 12, 12); glPopMatrix();

    // --- Belakang kepala (bola ikal) ---
    glPushMatrix(); glTranslatef(-0.18f, 0.98f,  0.35f); glutSolidSphere(0.150f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.00f, 0.96f,  0.38f); glutSolidSphere(0.165f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.18f, 0.98f,  0.35f); glutSolidSphere(0.150f, 12, 12); glPopMatrix();
    // Row bawah belakang
    glPushMatrix(); glTranslatef(-0.12f, 0.87f,  0.36f); glutSolidSphere(0.140f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.12f, 0.87f,  0.36f); glutSolidSphere(0.140f, 12, 12); glPopMatrix();

    // ---- 8c. RAMBUT BELAKANG KEPALA ----

    setColor(0.40f, 0.30f, 0.22f);
    glPushMatrix();
    glTranslatef(0.0f, 0.75f, 0.30f);
    glScalef(0.44f, 0.48f, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.56f, 0.29f);
    glScalef(0.34f, 0.20f, 0.07f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // ---- 9. RAMBUT / TOPI ----
    setColor(0.15f, 0.19f, 0.25f);
    glPushMatrix();
    glTranslatef(0.0f, 0.78f, 0.0f);
    // Brim topi
    glPushMatrix();
    glTranslatef(0.0f, 0.28f, -0.04f);
    glScalef(0.66f, 0.10f, 0.60f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Crown topi
    setColor(0.77f, 0.42f, 0.29f);
    glPushMatrix();
    glTranslatef(0.0f, 0.46f, 0.02f);
    glScalef(0.52f, 0.28f, 0.50f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix(); // end topi

    glPopMatrix(); // end player
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float camDistance = 5.5f; 
    float camHeight = 3.2f;   
    float rad = playerYaw * (PI / 180.0f);

    float eyeX = playerX + sin(rad) * camDistance;
    float eyeY = playerY + camHeight;
    float eyeZ = playerZ + cos(rad) * camDistance;

    gluLookAt(
        eyeX, eyeY, eyeZ,
        playerX, playerY + 0.8f, playerZ,
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

void keyboardDown(unsigned char key, int x, int y)
{
    keys[key] = true;
    if (key == 27) exit(0);
}

void keyboardUp(unsigned char key, int x, int y)
{
    keys[key] = false;
}

void update(int value)
{
    float rad = playerYaw * (PI / 180.0f);
    bool isWalking = false;

    if (keys['w']) {
        playerX -= sin(rad) * moveSpeed;
        playerZ -= cos(rad) * moveSpeed;
        isWalking = true;
    }
    if (keys['s']) {
        playerX += sin(rad) * moveSpeed;
        playerZ += cos(rad) * moveSpeed;
        isWalking = true;
    }
    if (keys['a']) playerYaw += turnSpeed;
    if (keys['d']) playerYaw -= turnSpeed;
    
    if (keys[' '] && isGrounded) {
        velocityY = jumpForce;
        isGrounded = false;
    }

    if (!isGrounded) {
        velocityY -= gravity;
    }
    playerY += velocityY;

    if (playerY <= groundY) {
        playerY = groundY;
        velocityY = 0.0f;
        isGrounded = true;
    }

    if (isGrounded) {
        if (isWalking) {
            walkAnim += 0.25f; 
            swingAngle = sin(walkAnim) * 42.0f; 
        } else {
            if (swingAngle > 0) swingAngle -= 2.0f;
            if (swingAngle < 0) swingAngle += 2.0f;
            if (fabs(swingAngle) < 2.0f) swingAngle = 0.0f; 
        }
    } else {
        swingAngle = 0.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); 
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Male 3D Player Mode");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}
