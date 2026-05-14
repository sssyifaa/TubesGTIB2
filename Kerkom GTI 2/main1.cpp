#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <vector>

static const float PI = 3.14159265f;
static const float D2R = PI / 180.0f;

// =============================================================================
//  Utilitas
// =============================================================================
static float rnd(float lo, float hi) {
    return lo + (hi - lo) * ((float)rand() / (float)RAND_MAX);
}
static float clampF(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
struct Color3 { float r, g, b; };
static Color3 makeColor(float r, float g, float b) {
    Color3 c; c.r=r; c.g=g; c.b=b; return c;
}
static Color3 shade(Color3 c, float f) {
    return makeColor(clampF(c.r*f,0,1), clampF(c.g*f,0,1), clampF(c.b*f,0,1));
}
static void setColor(Color3 c) { glColor3f(c.r,c.g,c.b); }
static void setColor(float r, float g, float b) { glColor3f(r,g,b); }

static void autoNormal(float ax,float ay,float az,
                        float bx,float by,float bz,
                        float cx,float cy,float cz) {
    float ux=bx-ax,uy=by-ay,uz=bz-az;
    float vx=cx-ax,vy=cy-ay,vz=cz-az;
    float nx=uy*vz-uz*vy, ny=uz*vx-ux*vz, nz=ux*vy-uy*vx;
    float len=sqrtf(nx*nx+ny*ny+nz*nz);
    if(len>0){nx/=len;ny/=len;nz/=len;}
    glNormal3f(nx,ny,nz);
}

static void quad4(float ax,float ay,float az,
                  float bx,float by,float bz,
                  float cx,float cy,float cz,
                  float dx,float dy,float dz) {
    autoNormal(ax,ay,az,bx,by,bz,cx,cy,cz);
    glBegin(GL_QUADS);
        glVertex3f(ax,ay,az); glVertex3f(bx,by,bz);
        glVertex3f(cx,cy,cz); glVertex3f(dx,dy,dz);
    glEnd();
}
static void tri3(float ax,float ay,float az,
                 float bx,float by,float bz,
                 float cx,float cy,float cz) {
    autoNormal(ax,ay,az,bx,by,bz,cx,cy,cz);
    glBegin(GL_TRIANGLES);
        glVertex3f(ax,ay,az); glVertex3f(bx,by,bz); glVertex3f(cx,cy,cz);
    glEnd();
}

// =============================================================================
//  Player (Karakter)
// =============================================================================
float playerX = 0.0f, playerY = 0.0f, playerZ = 12.0f;
float playerYaw = 180.0f;
float moveSpeed = 0.15f;
float turnSpeed = 3.0f;

float velocityY = 0.0f;
float gravity   = 0.012f;
float jumpForce = 0.22f;
float groundY   = 0.0f;
bool  isGrounded = true;

bool keys[256]        = {false};
bool specialKeys[256] = {false};

float walkAnim  = 0.0f;
float swingAngle = 0.0f;

// =============================================================================
// Tisya NPC 
// =============================================================================
float tisyaIdleTime = 0.0f;
float tisyaX = -22.0f;   // posisi X (bisa diubah)
float tisyaZ = -28.0f;   // posisi Z (bisa diubah)
float tisyaYaw = 180.0f; // arah hadap

// =============================================================================
//  State Sapi
// =============================================================================
static float cowX      = 0.f,  cowZ      = 0.f;
static float cowAngle  = 0.f;
static float walkPhase = 0.f;
static float tailPhase = 0.f;
static float udderSwing= 0.f;
static int   milkCount = 0;
static int   isMillking = 0;
static float milkAnim  = 0.f;

static float TONG_X = 0.f;
static float TONG_Z = 0.f;
static const float MILK_RANGE = 3.5f;

// =============================================================================
//  State Ayam
// =============================================================================
static float chickenX = 32.0f;
static float chickenZ = -35.0f;
static float chickenAngle = 0.0f;
static float chickenLegAnim = 0.0f;

// =============================================================================
//  Kamera
// =============================================================================
struct Camera {
    float distance, pitch, angleAroundPlayer;
    bool dragging; int lastX, lastY;
    Camera() : distance(6.0f), pitch(20.0f), angleAroundPlayer(0.0f),
               dragging(false), lastX(0), lastY(0) {}
    void apply() const {
        float px = playerX, py = playerY + 2.0f, pz = playerZ;
        float hDist = distance * cosf(pitch * D2R);
        float vDist = distance * sinf(pitch * D2R);
        float theta = (playerYaw + angleAroundPlayer) * D2R;
        float cx = px + hDist * sinf(theta);
        float cz = pz + hDist * cosf(theta);
        float cy = py + vDist;
        gluLookAt(cx, cy, cz, px, py, pz, 0, 1, 0);
    }
} cam;

// =============================================================================
//  Data Rumah
// =============================================================================
struct House {
    float cx,cz,width,depth,wallH,roofH,yardD;
    int side, type;
    Color3 wall,roof,door,win;
};
std::vector<House> houses;

static Color3 wallPal[] = {
    {0.86f,0.77f,0.62f},{0.91f,0.84f,0.72f},{0.79f,0.69f,0.56f},
    {0.93f,0.89f,0.81f},{0.72f,0.63f,0.52f},{0.82f,0.74f,0.64f}
};
static Color3 roofPal[] = {
    {0.48f,0.22f,0.12f},{0.55f,0.28f,0.14f},{0.36f,0.18f,0.10f},
    {0.30f,0.30f,0.30f},{0.20f,0.25f,0.30f}
};

void initHouses() {
    srand(42);
    float roadHalf=3.0f, startZ=4.0f, spacing=11.0f;
    int count=10;
    for(int i=0;i<count;i++){
        for(int s=-1;s<=1;s+=2){
            House h;
            h.side = s;
            h.type = rand() % 3;
            if(h.type==0){
                h.width=rnd(5.0f,6.0f); h.depth=rnd(5.5f,7.0f);
                h.wallH=rnd(2.8f,3.2f); h.roofH=rnd(1.8f,2.2f);
            } else if(h.type==1){
                h.width=rnd(4.5f,5.5f); h.depth=rnd(5.0f,6.0f);
                h.wallH=rnd(3.2f,3.8f); h.roofH=rnd(2.5f,3.5f);
            } else {
                h.width=rnd(7.0f,8.5f); h.depth=rnd(5.0f,6.5f);
                h.wallH=rnd(2.5f,2.9f); h.roofH=rnd(1.5f,2.0f);
            }
            h.yardD = rnd(1.5f,3.0f);
            h.cz    = startZ - i*spacing + rnd(-0.8f,0.8f);
            float dist = roadHalf + h.yardD + h.depth*0.5f + rnd(0,0.5f);
            h.cx   = s * dist;
            h.wall = wallPal[rand()%6];
            h.roof = roofPal[rand()%5];
            h.door = makeColor(0.32f,0.18f,0.08f);
            h.win  = makeColor(0.60f,0.80f,0.92f);
            houses.push_back(h);
        }
    }
}

// =============================================================================
//  Gambar Pagar
// =============================================================================
void drawFence(float x0,float z0,float x1,float z1,int posts) {
    float dx=(x1-x0)/posts, dz=(z1-z0)/posts;
    Color3 wood=makeColor(0.50f,0.34f,0.18f);
    float railH[2]={0.72f,0.36f};
    for(int r=0;r<2;r++){
        float th=0.035f;
        setColor(shade(wood, r==0?1.0f:0.85f));
        glBegin(GL_QUADS);
            glVertex3f(x0,railH[r]-th,z0); glVertex3f(x1,railH[r]-th,z1);
            glVertex3f(x1,railH[r]+th,z1); glVertex3f(x0,railH[r]+th,z0);
        glEnd();
    }
    for(int i=0;i<=posts;i++){
        float px=x0+dx*i, pz=z0+dz*i;
        float lit=0.78f+0.22f*(float)(i%2);
        setColor(shade(wood,lit));
        float tw=0.045f;
        glBegin(GL_QUADS);
            glVertex3f(px-tw,0,pz); glVertex3f(px+tw,0,pz);
            glVertex3f(px+tw,0.90f,pz); glVertex3f(px-tw,0.90f,pz);
        glEnd();
        setColor(shade(wood,lit*0.90f));
        glBegin(GL_TRIANGLES);
            glVertex3f(px-tw,0.90f,pz); glVertex3f(px+tw,0.90f,pz);
            glVertex3f(px, 1.06f,pz);
        glEnd();
    }
}

// =============================================================================
//  Kandang Sapi
// =============================================================================
void drawKandangSapi(float cx, float cz) {
    float w=8.0f, d=8.0f;
    float x0=cx-w/2, x1=cx+w/2;
    float z0=cz-d/2, z1=cz+d/2;

    drawFence(x0,z1,x1,z1,8);
    drawFence(x0,z0,x0,z1,8);
    drawFence(x1,z0,x1,z1,8);
    drawFence(x0,z0,x1,z0,8);

    setColor(0.38f,0.28f,0.18f);
    glBegin(GL_QUADS);
        glVertex3f(x0,0.001f,z0); glVertex3f(x1,0.001f,z0);
        glVertex3f(x1,0.001f,z1); glVertex3f(x0,0.001f,z1);
    glEnd();

    Color3 wood=makeColor(0.40f,0.25f,0.12f);
    setColor(wood);
    float tiangX[2] = {x0+0.3f, x1-0.3f};
    float tiangZ[2] = {z0+0.3f, z0+d*0.55f};
    for(int i=0;i<2;i++){
        for(int j=0;j<2;j++){
            glPushMatrix();
            glTranslatef(tiangX[i],0,tiangZ[j]);
            glScalef(0.2f,2.8f,0.2f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }
    setColor(0.65f,0.52f,0.28f);
    glPushMatrix();
    glTranslatef(cx, 2.85f, z0+d*0.28f);
    glScalef(w-0.2f, 0.22f, d*0.6f);
    glutSolidCube(1.0f);
    glPopMatrix();
    setColor(0.58f,0.46f,0.24f);
    glPushMatrix();
    glTranslatef(cx, 2.75f, z0+d*0.28f);
    glRotatef(12.0f,0,0,1);
    glScalef(w+0.3f, 0.15f, d*0.6f+0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// =============================================================================
//  Kandang Ayam
// =============================================================================
void drawKandangAyam(float cx, float cz) {
    float w=2.0f, d=2.0f;
    float x0=cx-w/2, x1=cx+w/2;
    float z0=cz-d/2, z1=cz+d/2;
    drawFence(x0,z1,x1,z1,6);
    drawFence(x0,z0,x0,z1,6);
    drawFence(x1,z0,x1,z1,6);
    drawFence(x0,z0,x1,z0,6);
    setColor(0.7f,0.3f,0.2f);
    glPushMatrix(); glTranslatef(cx,0.5f,z0+0.6f); glScalef(1.2f,1.0f,1.0f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.2f,0.2f,0.2f);
    glPushMatrix(); glTranslatef(cx,1.1f,z0+0.6f); glRotatef(45,1,0,0); glScalef(1.3f,0.8f,0.8f); glutSolidCube(1.0f); glPopMatrix();
}

// =============================================================================
//  Gambar Sapi
// =============================================================================
static float cylX(float r,float a){ return r*cosf(a); }
static float cylZ_(float r,float a){ return r*sinf(a); }

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

static void drawTong() {
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

static void drawCow() {
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

void drawSingleChicken(float x, float z, float angle, float legPhase) {
    glPushMatrix();
        glTranslatef(x, 0.4f, z);
        glRotatef(angle, 0, 1, 0);
        glScalef(0.5f, 0.5f, 0.5f);
        setColor(1.0f, 1.0f, 1.0f);
        glPushMatrix();
            glScalef(0.8f, 0.9f, 1.1f);
            glutSolidSphere(0.5, 20, 20);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(0, 0.4f, 0.4f);
            glutSolidSphere(0.35, 20, 20);
            setColor(1.0f, 0.5f, 0.0f);
            glPushMatrix();
                glTranslatef(0, -0.05f, 0.35f);
                glScalef(1, 0.8f, 1.5f);
                glutSolidCone(0.1, 0.2, 10, 10);
            glPopMatrix();
        glPopMatrix();
        setColor(1.0f, 0.8f, 0.0f);
        float s = sin(legPhase) * 30.0f;
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

// =============================================================================
//  Player
// =============================================================================
void drawPlayer() {
    glPushMatrix();
    glTranslatef(playerX, playerY+1.2f, playerZ);
    glRotatef(playerYaw,0.f,1.f,0.f);

    float armJumpPose = isGrounded ? 0.0f : -35.0f;

    glPushMatrix(); glTranslatef(-0.18f,-0.5f,0.0f); glRotatef(-swingAngle,1.f,0.f,0.f);
    setColor(0.77f,0.66f,0.51f);
    glPushMatrix(); glTranslatef(0,-0.55f,0); glScalef(0.30f,0.85f,0.30f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.55f,0.39f,0.25f);
    glPushMatrix(); glTranslatef(0,-1.0f,-0.08f); glScalef(0.32f,0.14f,0.42f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    glPushMatrix(); glTranslatef(0.18f,-0.5f,0.0f); glRotatef(swingAngle,1.f,0.f,0.f);
    setColor(0.77f,0.66f,0.51f);
    glPushMatrix(); glTranslatef(0,-0.55f,0); glScalef(0.30f,0.85f,0.30f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.55f,0.39f,0.25f);
    glPushMatrix(); glTranslatef(0,-1.0f,-0.08f); glScalef(0.32f,0.14f,0.42f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    setColor(0.18f,0.23f,0.29f);
    glPushMatrix(); glScalef(0.75f,0.95f,0.38f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.29f,0.39f,0.50f);
    glPushMatrix(); glTranslatef(0,0.3f,-0.20f); glScalef(0.28f,0.28f,0.04f); glutSolidCube(1.0f); glPopMatrix();

    setColor(0.77f,0.42f,0.29f);
    glPushMatrix(); glTranslatef(0,0.08f,0.22f); glScalef(0.55f,0.65f,0.18f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.85f,0.52f,0.37f);
    glPushMatrix(); glTranslatef(0,-0.08f,0.32f); glScalef(0.35f,0.32f,0.06f); glutSolidCube(1.0f); glPopMatrix();

    glPushMatrix(); glTranslatef(-0.48f,0.30f,0.0f); glRotatef(swingAngle+armJumpPose,1.f,0.f,0.f);
    setColor(0.18f,0.23f,0.29f);
    glPushMatrix(); glTranslatef(0,-0.30f,0); glScalef(0.22f,0.60f,0.22f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.76f,0.57f,0.37f);
    glPushMatrix(); glTranslatef(0,-0.68f,0); glutSolidSphere(0.10f,12,12); glPopMatrix();
    glPopMatrix();

    glPushMatrix(); glTranslatef(0.48f,0.30f,0.0f); glRotatef(-swingAngle+armJumpPose,1.f,0.f,0.f);
    setColor(0.18f,0.23f,0.29f);
    glPushMatrix(); glTranslatef(0,-0.30f,0); glScalef(0.22f,0.60f,0.22f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.76f,0.57f,0.37f);
    glPushMatrix(); glTranslatef(0,-0.68f,0); glutSolidSphere(0.10f,12,12); glPopMatrix();
    glPopMatrix();

    setColor(0.76f,0.57f,0.37f);
    glPushMatrix(); glTranslatef(0,0.53f,0); glScalef(0.20f,0.18f,0.20f); glutSolidCube(1.0f); glPopMatrix();

    setColor(0.76f,0.57f,0.37f);
    glPushMatrix();
    glTranslatef(0,0.78f,0);
    glScalef(0.58f,0.58f,0.54f);
    glutSolidCube(1.0f);
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

// =============================================================================
//  Tisya (NPC - karakter Tiskuy)
// =============================================================================
void drawTisya() {
    float armSwing = sinf(tisyaIdleTime) * 18.0f;

    glPushMatrix();
    glTranslatef(tisyaX, 1.2f, tisyaZ);
    glRotatef(tisyaYaw, 0.0f, 1.0f, 0.0f);

    // Kaki Kiri
    setColor(0.91f, 0.87f, 0.82f);
    glPushMatrix(); glTranslatef(-0.14f, 0.1f, 0.0f);
    glPushMatrix(); glTranslatef(0.0f, -0.55f, 0.0f); glScalef(0.28f, 0.85f, 0.28f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.48f, 0.29f, 0.31f);
    glPushMatrix(); glTranslatef(0.0f, -1.0f, -0.06f); glScalef(0.28f, 0.13f, 0.38f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    // Kaki Kanan
    setColor(0.91f, 0.87f, 0.82f);
    glPushMatrix(); glTranslatef(0.14f, 0.1f, 0.0f);
    glPushMatrix(); glTranslatef(0.0f, -0.55f, 0.0f); glScalef(0.28f, 0.85f, 0.28f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.48f, 0.29f, 0.31f);
    glPushMatrix(); glTranslatef(0.0f, -1.0f, -0.06f); glScalef(0.28f, 0.13f, 0.38f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();

    // Rok
    setColor(0.91f, 0.87f, 0.82f);
    glPushMatrix(); glTranslatef(0.0f, -0.10f, 0.0f); glScalef(0.78f, 0.35f, 0.40f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.91f, 0.87f, 0.82f);
    glPushMatrix(); glTranslatef(0.0f, -0.33f, 0.0f); glScalef(0.90f, 0.22f, 0.46f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.69f, 0.44f, 0.38f);
    glPushMatrix(); glTranslatef(0.0f, -0.44f, 0.0f); glScalef(0.91f, 0.04f, 0.47f); glutSolidCube(1.0f); glPopMatrix();

    // Badan (blouse) - warna beda dari Tiskuy: biru muda
    setColor(0.52f, 0.72f, 0.88f);
    glPushMatrix(); glScalef(0.70f, 0.90f, 0.36f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.95f, 0.92f, 0.88f);
    glPushMatrix(); glTranslatef( 0.10f, 0.32f, -0.19f); glScalef(0.14f, 0.22f, 0.04f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.10f, 0.32f, -0.19f); glScalef(0.14f, 0.22f, 0.04f); glutSolidCube(1.0f); glPopMatrix();

    // Tangan Kiri
    glPushMatrix();
    glTranslatef(-0.44f, 0.28f, 0.0f);
    glRotatef(armSwing, 1.0f, 0.0f, 0.0f);
    setColor(0.52f, 0.72f, 0.88f);
    glPushMatrix(); glTranslatef(0.0f, -0.28f, 0.0f); glScalef(0.20f, 0.56f, 0.20f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.76f, 0.57f, 0.38f);
    glPushMatrix(); glTranslatef(0.0f, -0.64f, 0.0f); glutSolidSphere(0.095f, 12, 12); glPopMatrix();
    glPopMatrix();

    // Tangan Kanan
    glPushMatrix();
    glTranslatef(0.44f, 0.28f, 0.0f);
    glRotatef(-armSwing, 1.0f, 0.0f, 0.0f);
    setColor(0.52f, 0.72f, 0.88f);
    glPushMatrix(); glTranslatef(0.0f, -0.28f, 0.0f); glScalef(0.20f, 0.56f, 0.20f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.76f, 0.57f, 0.38f);
    glPushMatrix(); glTranslatef(0.0f, -0.64f, 0.0f); glutSolidSphere(0.095f, 12, 12); glPopMatrix();
    glPopMatrix();

    // Leher
    setColor(0.76f, 0.57f, 0.38f);
    glPushMatrix(); glTranslatef(0.0f, 0.52f, 0.0f); glScalef(0.18f, 0.16f, 0.18f); glutSolidCube(1.0f); glPopMatrix();

    // Kepala
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

    // Rambut bondol (sama persis Tiskuy)
    setColor(0.16f, 0.15f, 0.15f);
    glPushMatrix(); glTranslatef(0.0f, 1.06f, 0.02f); glScalef(0.60f, 0.18f, 0.56f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.32f, 0.84f, 0.00f); glScalef(0.10f, 0.44f, 0.50f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.32f, 0.84f, 0.00f); glScalef(0.10f, 0.44f, 0.50f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.84f, 0.30f); glScalef(0.56f, 0.46f, 0.10f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.99f, -0.24f); glScalef(0.52f, 0.12f, 0.10f); glutSolidCube(1.0f); glPopMatrix();

    glPopMatrix();
}

// =============================================================================
//  Pohon
// =============================================================================
void drawTreeNatural(float tx, float tz, unsigned int seed, bool hasApples=false) {
    srand(seed);
    float trunkH=rnd(2.5f,4.5f), rBot=0.13f+rnd(0,0.04f), rTop=0.05f+rnd(0,0.02f);
    Color3 bark=makeColor(0.30f+rnd(-0.05f,0.05f),0.20f+rnd(-0.03f,0.03f),0.10f+rnd(-0.02f,0.02f));
    int cseg=10;
    float hLevels[4]={0,trunkH*0.35f,trunkH*0.72f,trunkH};
    float rLevels[4]={rBot,rBot*0.82f,rTop*1.4f,rTop};
    for(int s=0;s<3;s++){
        float ys0=hLevels[s],ys1=hLevels[s+1],rs0=rLevels[s],rs1=rLevels[s+1];
        for(int i=0;i<cseg;i++){
            float a0=(float)i/cseg*2*PI,a1=(float)(i+1)/cseg*2*PI;
            float lit=0.70f+0.30f*(cosf((a0+a1)*0.5f)*0.5f+0.5f);
            setColor(shade(bark,lit));
            glBegin(GL_QUADS);
                glNormal3f(cosf(a0),0.15f,sinf(a0));
                glVertex3f(tx+cylX(rs0,a0),ys0,tz+cylZ_(rs0,a0));
                glVertex3f(tx+cylX(rs1,a0),ys1,tz+cylZ_(rs1,a0));
                glVertex3f(tx+cylX(rs1,a1),ys1,tz+cylZ_(rs1,a1));
                glVertex3f(tx+cylX(rs0,a1),ys0,tz+cylZ_(rs0,a1));
            glEnd();
        }
    }
    Color3 leafBase = hasApples
        ? makeColor(0.2f,0.45f,0.1f)
        : makeColor(0.16f+rnd(-0.04f,0.04f),0.40f+rnd(-0.06f,0.06f),0.12f+rnd(-0.03f,0.03f));
    float crownBase=trunkH*0.38f,crownR=rnd(1.2f,2.0f);
    int layers=5;
    for(int L=0;L<layers;L++){
        float lf=(float)L/(layers-1),layerY=crownBase+lf*(trunkH*0.65f),offX=rnd(-0.2f,0.2f);
        float layerR=crownR*(1.0f-lf*0.55f)*rnd(0.85f,1.15f),layerH=crownR*0.60f*(1.0f-lf*0.28f);
        Color3 lColor=makeColor(clampF(leafBase.r+0.04f*L,0,1),clampF(leafBase.g+0.05f*L,0,1),clampF(leafBase.b+0.02f*L,0,1));
        int lseg=14;
        for(int i=0;i<lseg;i++){
            float a0=(float)i/lseg*2*PI,a1=(float)(i+1)/lseg*2*PI,am=(a0+a1)*0.5f;
            float sunLit=0.72f+0.28f*(cosf(am)*0.55f+0.45f);
            setColor(shade(lColor,sunLit));
            glBegin(GL_TRIANGLES);
                glNormal3f(cosf(am),0.50f,sinf(am));
                glVertex3f(tx+offX+layerR*cosf(a0),layerY,tz+layerR*sinf(a0));
                glVertex3f(tx+offX+layerR*cosf(a1),layerY,tz+layerR*sinf(a1));
                glVertex3f(tx+offX,layerY+layerH,tz);
            glEnd();
            if(hasApples && (rand()%100<25)){
                setColor(makeColor(0.85f,0.1f,0.1f));
                glPushMatrix();
                glTranslatef(tx+offX+layerR*cosf(am)*0.85f,layerY+layerH*0.3f,tz+layerR*sinf(am)*0.85f);
                glutSolidSphere(0.12f,10,10);
                glPopMatrix();
            }
        }
        setColor(shade(lColor,0.55f));
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0,-1,0);
        glVertex3f(tx+offX,layerY,tz);
        for(int i=0;i<=lseg;i++){float a=(float)i/lseg*2*PI;glVertex3f(tx+offX+layerR*cosf(a),layerY,tz+layerR*sinf(a));}
        glEnd();
    }
}

// =============================================================================
// Gambar Dinding Bata Procedural
// =============================================================================
void drawBrickWallX(float fx, float z0, float z1, float y0, float y1,
                    Color3 baseColor, float normalDir)
{
    float brickH  = 0.18f;   // tinggi satu bata
    float brickL  = 0.38f;   // panjang satu bata
    float mortarV = 0.025f;  // celah mortar vertikal
    float mortarH = 0.020f;  // celah mortar horizontal
    Color3 mortarCol = makeColor(0.70f,0.68f,0.64f);
    Color3 brickDark = shade(baseColor, 0.82f);
    Color3 brickLight= shade(baseColor, 1.10f);

    float depth = 0.008f; 

    float panelZ = z1 - z0;
    float panelY = y1 - y0;

    glDisable(GL_LIGHTING);
    setColor(mortarCol);
    glNormal3f(normalDir, 0.f, 0.f);
    glBegin(GL_QUADS);
        glVertex3f(fx + normalDir*depth, y0, z0);
        glVertex3f(fx + normalDir*depth, y0, z1);
        glVertex3f(fx + normalDir*depth, y1, z1);
        glVertex3f(fx + normalDir*depth, y1, z0);
    glEnd();
    glEnable(GL_LIGHTING);

    int nRows = (int)((panelY) / (brickH + mortarV)) + 1;
    int nCols = (int)((panelZ) / (brickL + mortarH)) + 2;

    for(int row = 0; row < nRows; row++){
        float yBot = y0 + row * (brickH + mortarV);
        float yTop = yBot + brickH;
        if(yTop > y1) yTop = y1;
        if(yBot >= y1) break;

        float offset = (row % 2 == 0) ? 0.f : (brickL + mortarH) * 0.5f;

        for(int col = -1; col < nCols; col++){
            float zLeft  = z0 + col*(brickL + mortarH) - offset + mortarH*0.5f;
            float zRight = zLeft + brickL;

            if(zRight <= z0) continue;
            if(zLeft  >= z1) break;
            float cl = (zLeft  < z0) ? z0 : zLeft;
            float cr = (zRight > z1) ? z1 : zRight;

            float var = 0.92f + 0.16f * (float)((row*13 + col*7) % 8) / 7.f;
            Color3 bc = (row % 3 == 0) ? shade(brickDark, var) : shade(brickLight, var);
            setColor(bc);

            float bDepth = depth + 0.002f;
            glNormal3f(normalDir, 0.f, 0.f);
            glBegin(GL_QUADS);
                glVertex3f(fx + normalDir*bDepth, yBot, cl);
                glVertex3f(fx + normalDir*bDepth, yBot, cr);
                glVertex3f(fx + normalDir*bDepth, yTop, cr);
                glVertex3f(fx + normalDir*bDepth, yTop, cl);
            glEnd();
        }
    }
}

void drawBrickWallZ(float fz, float x0, float x1, float y0, float y1,
                    Color3 baseColor, float normalDir)
{
    float brickH  = 0.18f;
    float brickL  = 0.38f;
    float mortarV = 0.025f;
    float mortarH = 0.020f;
    Color3 mortarCol = makeColor(0.70f,0.68f,0.64f);
    Color3 brickDark = shade(baseColor, 0.82f);
    Color3 brickLight= shade(baseColor, 1.10f);

    float depth = 0.008f;

    float panelX = x1 - x0;
    float panelY = y1 - y0;

    glDisable(GL_LIGHTING);
    setColor(mortarCol);
    glNormal3f(0.f, 0.f, normalDir);
    glBegin(GL_QUADS);
        glVertex3f(x0, y0, fz + normalDir*depth);
        glVertex3f(x1, y0, fz + normalDir*depth);
        glVertex3f(x1, y1, fz + normalDir*depth);
        glVertex3f(x0, y1, fz + normalDir*depth);
    glEnd();
    glEnable(GL_LIGHTING);

    int nRows = (int)((panelY) / (brickH + mortarV)) + 1;
    int nCols = (int)((panelX) / (brickL + mortarH)) + 2;

    for(int row = 0; row < nRows; row++){
        float yBot = y0 + row * (brickH + mortarV);
        float yTop = yBot + brickH;
        if(yTop > y1) yTop = y1;
        if(yBot >= y1) break;

        float offset = (row % 2 == 0) ? 0.f : (brickL + mortarH) * 0.5f;

        for(int col = -1; col < nCols; col++){
            float xLeft  = x0 + col*(brickL + mortarH) - offset + mortarH*0.5f;
            float xRight = xLeft + brickL;
            if(xRight <= x0) continue;
            if(xLeft  >= x1) break;
            float cl = (xLeft  < x0) ? x0 : xLeft;
            float cr = (xRight > x1) ? x1 : xRight;

            float var = 0.92f + 0.16f * (float)((row*13 + col*7) % 8) / 7.f;
            Color3 bc = (row % 3 == 0) ? shade(brickDark, var) : shade(brickLight, var);
            setColor(bc);

            float bDepth = depth + 0.002f;
            glNormal3f(0.f, 0.f, normalDir);
            glBegin(GL_QUADS);
                glVertex3f(cl, yBot, fz + normalDir*bDepth);
                glVertex3f(cr, yBot, fz + normalDir*bDepth);
                glVertex3f(cr, yTop, fz + normalDir*bDepth);
                glVertex3f(cl, yTop, fz + normalDir*bDepth);
            glEnd();
        }
    }
}

// =============================================================================
//  Jendela & Pintu
// =============================================================================
void drawWindow(float wx0,float wy0,float wz,float wx1,float wy1,Color3 winC,float normalZ){
    setColor(winC);
    glBegin(GL_QUADS);
        glNormal3f(0,0,normalZ);
        glVertex3f(wx0,wy0,wz); glVertex3f(wx1,wy0,wz);
        glVertex3f(wx1,wy1,wz); glVertex3f(wx0,wy1,wz);
    glEnd();
    Color3 fr=makeColor(0.50f,0.40f,0.25f); float fw=0.03f; setColor(fr);
    glBegin(GL_QUADS);
        glVertex3f(wx0-fw,wy1,wz+0.005f); glVertex3f(wx1+fw,wy1,wz+0.005f); glVertex3f(wx1+fw,wy1+fw,wz+0.005f); glVertex3f(wx0-fw,wy1+fw,wz+0.005f);
        glVertex3f(wx0-fw,wy0-fw,wz+0.005f); glVertex3f(wx1+fw,wy0-fw,wz+0.005f); glVertex3f(wx1+fw,wy0,wz+0.005f); glVertex3f(wx0-fw,wy0,wz+0.005f);
        glVertex3f(wx0-fw,wy0-fw,wz+0.005f); glVertex3f(wx0,wy0-fw,wz+0.005f); glVertex3f(wx0,wy1+fw,wz+0.005f); glVertex3f(wx0-fw,wy1+fw,wz+0.005f);
        glVertex3f(wx1,wy0-fw,wz+0.005f); glVertex3f(wx1+fw,wy0-fw,wz+0.005f); glVertex3f(wx1+fw,wy1+fw,wz+0.005f); glVertex3f(wx1,wy1+fw,wz+0.005f);
    glEnd();
}

void drawWindowX(float fx, float z0, float z1, float y0, float y1,
                 Color3 winC, float normalDir)
{
    float eps = 0.015f;
    setColor(winC);
    glNormal3f(normalDir,0,0);
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*eps, y0, z0);
        glVertex3f(fx+normalDir*eps, y0, z1);
        glVertex3f(fx+normalDir*eps, y1, z1);
        glVertex3f(fx+normalDir*eps, y1, z0);
    glEnd();
    // Bingkai
    Color3 fr=makeColor(0.50f,0.40f,0.25f); float fw=0.03f; setColor(fr);
    float e2 = eps+0.005f;
    // atas
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*e2,y1,    z0-fw); glVertex3f(fx+normalDir*e2,y1,    z1+fw);
        glVertex3f(fx+normalDir*e2,y1+fw, z1+fw); glVertex3f(fx+normalDir*e2,y1+fw, z0-fw);
    glEnd();
    // bawah
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*e2,y0-fw, z0-fw); glVertex3f(fx+normalDir*e2,y0-fw, z1+fw);
        glVertex3f(fx+normalDir*e2,y0,    z1+fw); glVertex3f(fx+normalDir*e2,y0,    z0-fw);
    glEnd();
    // kiri
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*e2,y0-fw, z0-fw); glVertex3f(fx+normalDir*e2,y0-fw, z0);
        glVertex3f(fx+normalDir*e2,y1+fw, z0);    glVertex3f(fx+normalDir*e2,y1+fw, z0-fw);
    glEnd();
    // kanan
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*e2,y0-fw, z1);    glVertex3f(fx+normalDir*e2,y0-fw, z1+fw);
        glVertex3f(fx+normalDir*e2,y1+fw, z1+fw); glVertex3f(fx+normalDir*e2,y1+fw, z1);
    glEnd();
}

// =============================================================================
//  Rumah
// =============================================================================
void drawHouse(const House &h){
    float x0=h.cx-h.width*0.5f, x1=h.cx+h.width*0.5f;
    float z0=h.cz-h.depth*0.5f, z1=h.cz+h.depth*0.5f;
    float y0=0.f, y1=h.wallH;
    float ov=0.35f;

    Color3 wF=h.wall, wS=shade(h.wall,0.80f), wB=shade(h.wall,0.65f);

    // -----------------------------------------------------------------------
    // 1. Dinding 
    // -----------------------------------------------------------------------
    setColor(wF); quad4(x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y1,z1); 
    setColor(wB); quad4(x1,y0,z0, x0,y0,z0, x0,y1,z0, x1,y1,z0); 
    setColor(wS); quad4(x0,y0,z0, x0,y0,z1, x0,y1,z1, x0,y1,z0); 
    setColor(shade(h.wall,0.87f));
              quad4(x1,y0,z1, x1,y0,z0, x1,y1,z0, x1,y1,z1); 

    // -----------------------------------------------------------------------
    // 2. Tekstur Bata 
    // -----------------------------------------------------------------------
    drawBrickWallZ(z1, x0, x1, y0, y1, wF, +1.f);
    drawBrickWallZ(z0, x0, x1, y0, y1, wB, -1.f);
    drawBrickWallX(x0, z0, z1, y0, y1, wS, -1.f);
    drawBrickWallX(x1, z0, z1, y0, y1, shade(h.wall,0.87f), +1.f);

    // -----------------------------------------------------------------------
    // 3. Pintu & Jendela
    // -----------------------------------------------------------------------
    float dw  = h.width * 0.16f; 
    float dh  = h.wallH * 0.52f; 
    float ww  = h.width * 0.15f;
    float wh  = h.wallH * 0.27f; 
    float wb  = y0 + h.wallH*0.50f;
    float wt  = wb + wh;   

    if(h.side == -1){
        float fx  = x1;
        float nDir= +1.f;
        float eps = 0.02f;

        float dzMid = (z0 + z1) * 0.5f;
        setColor(h.door);
        glNormal3f(nDir,0,0);
        glBegin(GL_QUADS);
            glVertex3f(fx+nDir*eps, y0,  dzMid-dw*0.5f);
            glVertex3f(fx+nDir*eps, y0,  dzMid+dw*0.5f);
            glVertex3f(fx+nDir*eps, dh,  dzMid+dw*0.5f);
            glVertex3f(fx+nDir*eps, dh,  dzMid-dw*0.5f);
        glEnd();

        setColor(makeColor(0.50f,0.40f,0.25f));
        float pf=0.04f;
        glBegin(GL_QUADS);
            glVertex3f(fx+nDir*(eps+0.005f),dh,    dzMid-dw*0.5f-pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh,    dzMid+dw*0.5f+pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh+pf, dzMid+dw*0.5f+pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh+pf, dzMid-dw*0.5f-pf);
        glEnd();

        float pz[2] = { dzMid - h.depth*0.26f, dzMid + h.depth*0.26f };
        for(int i=0;i<2;i++){
            drawWindowX(fx, pz[i]-ww*0.5f, pz[i]+ww*0.5f, wb, wt, h.win, nDir);
        }
    } else {
        float fx  = x0;
        float nDir= -1.f;
        float eps = 0.02f;

        float dzMid = (z0 + z1) * 0.5f;
        setColor(h.door);
        glNormal3f(nDir,0,0);
        glBegin(GL_QUADS);
            glVertex3f(fx+nDir*eps, y0, dzMid-dw*0.5f);
            glVertex3f(fx+nDir*eps, y0, dzMid+dw*0.5f);
            glVertex3f(fx+nDir*eps, dh, dzMid+dw*0.5f);
            glVertex3f(fx+nDir*eps, dh, dzMid-dw*0.5f);
        glEnd();
        setColor(makeColor(0.50f,0.40f,0.25f));
        float pf=0.04f;
        glBegin(GL_QUADS);
            glVertex3f(fx+nDir*(eps+0.005f),dh,    dzMid-dw*0.5f-pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh,    dzMid+dw*0.5f+pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh+pf, dzMid+dw*0.5f+pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh+pf, dzMid-dw*0.5f-pf);
        glEnd();

        float pz[2] = { dzMid - h.depth*0.26f, dzMid + h.depth*0.26f };
        for(int i=0;i<2;i++){
            drawWindowX(fx, pz[i]-ww*0.5f, pz[i]+ww*0.5f, wb, wt, h.win, nDir);
        }
    }

    // -----------------------------------------------------------------------
    // 4. Atap 
    // -----------------------------------------------------------------------
    float rx0=x0-ov, rx1=x1+ov, rz0=z0-ov, rz1=z1+ov, rY=y1+h.roofH, rX=h.cx;
    setColor(h.roof);          quad4(rx0,y1,rz1, rx0,y1,rz0, rX,rY,rz0, rX,rY,rz1);
    setColor(shade(h.roof,0.68f)); quad4(rx1,y1,rz0, rx1,y1,rz1, rX,rY,rz1, rX,rY,rz0);
    setColor(shade(h.wall,0.92f)); tri3(rx0,y1,rz1, rx1,y1,rz1, rX,rY,rz1);
    setColor(shade(h.wall,0.72f)); tri3(rx1,y1,rz0, rx0,y1,rz0, rX,rY,rz0);
}

// =============================================================================
//  Jalan, Tanah, Langit
// =============================================================================
void drawRoad(){
    float rw=6.0f,zN=20.0f,zF=-150.0f,sw=1.4f;
    glColor3f(0.20f,0.20f,0.20f); glNormal3f(0,1,0);
    glBegin(GL_QUADS); glVertex3f(-rw*.5f,0.005f,zN); glVertex3f(rw*.5f,0.005f,zN); glVertex3f(rw*.5f,0.005f,zF); glVertex3f(-rw*.5f,0.005f,zF); glEnd();
    glColor3f(0.40f,0.34f,0.25f); glNormal3f(0,1,0);
    for(int s=-1;s<=1;s+=2){
        float sx0=s*rw*.5f,sx1=s*(rw*.5f+sw);
        glBegin(GL_QUADS); glVertex3f(sx0,0.003f,zN); glVertex3f(sx1,0.003f,zN); glVertex3f(sx1,0.003f,zF); glVertex3f(sx0,0.003f,zF); glEnd();
    }
}

void drawGround(){
    glColor3f(0.30f,0.25f,0.18f); glNormal3f(0,1,0);
    glBegin(GL_QUADS); glVertex3f(-300,0,25); glVertex3f(300,0,25); glVertex3f(300,0,-200); glVertex3f(-300,0,-200); glEnd();
    glColor3f(0.24f,0.38f,0.16f); glNormal3f(0,1,0);
    glBegin(GL_QUADS); glVertex3f(-100,0.002f,22); glVertex3f(100,0.002f,22); glVertex3f(100,0.002f,-180); glVertex3f(-100,0.002f,-180); glEnd();
}

void drawSky(){
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    float zS=-180.0f, xW=300.0f;
    struct SkyBand{float y0,y1,r0,g0,b0,r1,g1,b1;};
    SkyBand bands[]={
        {0, 8,  0.72f,0.62f,0.55f, 0.65f,0.55f,0.60f},
        {8, 20, 0.65f,0.55f,0.60f, 0.45f,0.50f,0.70f},
        {20,45, 0.45f,0.50f,0.70f, 0.25f,0.35f,0.65f},
    };
    for(int i=0;i<3;i++){
        SkyBand &b=bands[i];
        glBegin(GL_QUADS);
            glColor3f(b.r0,b.g0,b.b0); glVertex3f(-xW,b.y0,zS); glVertex3f(xW,b.y0,zS);
            glColor3f(b.r1,b.g1,b.b1); glVertex3f(xW,b.y1,zS);  glVertex3f(-xW,b.y1,zS);
        glEnd();
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// =============================================================================
//  Scene Utama
// =============================================================================
void drawScene(){
    drawSky();
    drawGround();
    drawRoad();

    for(int i=0;i<(int)houses.size();i++)
        drawHouse(houses[i]);

    srand(123);

    for(int i=0;i<(int)houses.size();i++){
        const House &h=houses[i];
        float tz=h.cz-(h.depth*0.5f)-rnd(3.0f,5.0f);
        float tx=h.cx+rnd(-h.width*0.4f,h.width*0.4f);
        drawTreeNatural(tx,tz,(unsigned)(i*9+3));
    }

    {
        float kx =  22.0f;
        float kz = -35.0f;
        TONG_X = kx + 2.5f;
        TONG_Z = kz + 1.0f;
        drawKandangSapi(kx, kz);
        drawTong();
        drawCow();
        drawTreeNatural(kx - 6.5f, kz - 5.0f, 901);
        drawTreeNatural(kx + 5.5f, kz - 6.0f, 902);
        drawTreeNatural(kx - 6.0f, kz + 4.5f, 903);
    }

    {
        float ax_area = 32.0f;
        float az_area = -35.0f;
        float w = 8.0f;
        float d = 8.0f;
        drawFence(ax_area - w/2, az_area + d/2, ax_area + w/2, az_area + d/2, 8);
        drawFence(ax_area - w/2, az_area - d/2, ax_area - w/2, az_area + d/2, 8);
        drawFence(ax_area + w/2, az_area - d/2, ax_area + w/2, az_area + d/2, 8);
        drawFence(ax_area - w/2, az_area - d/2, ax_area + w/2, az_area - d/2, 8);
        setColor(0.35f, 0.45f, 0.1f);
        glBegin(GL_QUADS);
            glVertex3f(ax_area - w/2, 0.001f, az_area - d/2);
            glVertex3f(ax_area + w/2, 0.001f, az_area - d/2);
            glVertex3f(ax_area + w/2, 0.001f, az_area + d/2);
            glVertex3f(ax_area - w/2, 0.001f, az_area + d/2);
        glEnd();
        drawSingleChicken(chickenX, chickenZ, chickenAngle, chickenLegAnim);
    }

    srand(55);
    for(int row=0;row<4;row++){
        for(int col=0;col<5;col++){
            float ax = -15.0f - col*5.0f + rnd(-0.5f,0.5f);
            float az = -20.0f - row*6.0f + rnd(-0.5f,0.5f);
            drawTreeNatural(ax,az,(unsigned)(row*10+col),true);
        }
    }

    for(int i=0;i<(int)houses.size();i+=2){
        const House &h=houses[i];
        drawFence(h.cx-h.width*0.5f, h.cz+h.depth*0.5f+0.08f,
                  h.cx+h.width*0.5f, h.cz+h.depth*0.5f+0.08f, 8);
    }

    drawPlayer();
    drawTisya();
}

// =============================================================================
//  Setup Pencahayaan & Fog
// =============================================================================
void setupEnvironment(){
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    GLfloat sunPos[]={-15,12,10,0};
    GLfloat sunAmb[]={0.25f,0.20f,0.15f,1};
    GLfloat sunDif[]={1.00f,0.85f,0.65f,1};
    GLfloat sunSpc[]={0.30f,0.25f,0.20f,1};
    glLightfv(GL_LIGHT0,GL_POSITION,sunPos);
    glLightfv(GL_LIGHT0,GL_AMBIENT, sunAmb);
    glLightfv(GL_LIGHT0,GL_DIFFUSE, sunDif);
    glLightfv(GL_LIGHT0,GL_SPECULAR,sunSpc);

    GLfloat skyPos[]={10,20,-10,0};
    GLfloat skyDif[]={0.20f,0.25f,0.35f,1};
    GLfloat skyAmb[]={0.10f,0.15f,0.20f,1};
    GLfloat noSpec[]={0,0,0,1};
    glLightfv(GL_LIGHT1,GL_POSITION,skyPos);
    glLightfv(GL_LIGHT1,GL_DIFFUSE, skyDif);
    glLightfv(GL_LIGHT1,GL_AMBIENT, skyAmb);
    glLightfv(GL_LIGHT1,GL_SPECULAR,noSpec);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);

    GLfloat matSpc[]={0.05f,0.05f,0.05f,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,matSpc);
    glMaterialf (GL_FRONT_AND_BACK,GL_SHININESS,5.0f);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE,GL_EXP2);
    GLfloat fogColor[]={0.72f,0.62f,0.55f,1.0f};
    glFogfv(GL_FOG_COLOR,fogColor);
    glFogf(GL_FOG_DENSITY,0.012f);
    glHint(GL_FOG_HINT,GL_NICEST);
}

// =============================================================================
//  GLUT Callbacks
// =============================================================================
void display(){
    glClearColor(0.72f,0.62f,0.55f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    cam.apply();
    setupEnvironment();
    drawScene();
    glutSwapBuffers();
}

void reshape(int w,int h){
    if(!h) h=1;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(50.0f,(float)w/h,0.05f,400.0f);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key,int,int){
    keys[key]=true;
    if(key==27) exit(0);
    if(key=='r'||key=='R'){
        playerX=0.0f; playerY=groundY; playerZ=12.0f;
        playerYaw=180.0f;
        velocityY=0.0f; isGrounded=true;
        cam.angleAroundPlayer=0.0f; cam.pitch=20.0f; cam.distance=6.0f;
    }
}

void keyboardUp(unsigned char key,int,int){ keys[key]=false; }
void specialKey(int key,int,int){ specialKeys[key]=true; }
void specialKeyUp(int key,int,int){ specialKeys[key]=false; }

void mouseBtn(int btn,int state,int x,int y){
    if(btn==GLUT_LEFT_BUTTON){ cam.dragging=(state==GLUT_DOWN); cam.lastX=x; cam.lastY=y; }
    if(btn==3){ cam.distance-=0.5f; glutPostRedisplay(); }
    if(btn==4){ cam.distance+=0.5f; glutPostRedisplay(); }
    if(cam.distance<1.5f) cam.distance=1.5f;
}

void mouseMove(int x,int y){
    if(!cam.dragging) return;
    int dx=x-cam.lastX, dy=y-cam.lastY;
    cam.lastX=x; cam.lastY=y;
    cam.angleAroundPlayer -= dx*0.4f;
    cam.pitch              -= dy*0.4f;
    if(cam.pitch> 80.0f) cam.pitch= 80.0f;
    if(cam.pitch<-10.0f) cam.pitch=-10.0f;
    glutPostRedisplay();
}

void update(int value){
    float rad=playerYaw*(PI/180.0f);
    bool isWalking=false;

    bool moveForward  = keys['w']||keys['W']||specialKeys[GLUT_KEY_UP];
    bool moveBackward = keys['s']||keys['S']||specialKeys[GLUT_KEY_DOWN];
    bool turnLeft     = keys['a']||keys['A']||specialKeys[GLUT_KEY_LEFT];
    bool turnRight    = keys['d']||keys['D']||specialKeys[GLUT_KEY_RIGHT];

    if(moveForward) { playerX-=sinf(rad)*moveSpeed; playerZ-=cosf(rad)*moveSpeed; isWalking=true; }
    if(moveBackward){ playerX+=sinf(rad)*moveSpeed; playerZ+=cosf(rad)*moveSpeed; isWalking=true; }
    if(turnLeft)  playerYaw+=turnSpeed;
    if(turnRight) playerYaw-=turnSpeed;

    if(keys[' ']&&isGrounded){ velocityY=jumpForce; isGrounded=false; }
    if(!isGrounded) velocityY-=gravity;
    playerY+=velocityY;
    if(playerY<=groundY){ playerY=groundY; velocityY=0.0f; isGrounded=true; }

    if(isGrounded){
        if(isWalking){ walkAnim+=0.25f; swingAngle=sinf(walkAnim)*42.0f; }
        else{
            if(swingAngle> 0) swingAngle-=2.0f;
            if(swingAngle< 0) swingAngle+=2.0f;
            if(fabsf(swingAngle)<2.0f) swingAngle=0.0f;
        }
    } else { swingAngle=0.0f; }

    tailPhase+=0.033f*2.5f;
    
    tisyaIdleTime += 0.035f;
    
    if(isMillking){
        udderSwing+=0.033f*8.f;
        milkAnim  +=0.033f*6.f;
        if(milkAnim>3.f){ milkAnim=0.f; udderSwing=0.f; isMillking=0; milkCount++; }
    }

    glutPostRedisplay();
    glutTimerFunc(16,update,0);

    float aSpd = 0.04f;
    float chickenLimitX_Min = 28.5f, chickenLimitX_Max = 35.5f;
    float chickenLimitZ_Min = -38.5f, chickenLimitZ_Max = -31.5f;

    float nextCX = chickenX + sinf(chickenAngle * D2R) * aSpd;
    float nextCZ = chickenZ + cosf(chickenAngle * D2R) * aSpd;

    if (nextCX > chickenLimitX_Max || nextCX < chickenLimitX_Min ||
        nextCZ > chickenLimitZ_Max || nextCZ < chickenLimitZ_Min) {
        chickenAngle += 90.0f;
    } else {
        chickenX = nextCX;
        chickenZ = nextCZ;
        chickenLegAnim += 0.15f;
    }
}

// =============================================================================
//  Main
// =============================================================================
void init(){
    glClearColor(0.72f,0.77f,0.83f,1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    float lightPos[]={8.0f,15.0f,6.0f,1.0f};
    float ambient[] ={0.45f,0.42f,0.40f,1.0f};
    float diffuse[] ={0.90f,0.88f,0.82f,1.0f};
    glLightfv(GL_LIGHT0,GL_POSITION,lightPos);
    glLightfv(GL_LIGHT0,GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE, diffuse);
    float lightPos2[]={-5.0f,3.0f,-8.0f,1.0f};
    float ambient2[] ={0.20f,0.22f,0.25f,1.0f};
    float diffuse2[] ={0.25f,0.28f,0.32f,1.0f};
    glLightfv(GL_LIGHT1,GL_POSITION,lightPos2);
    glLightfv(GL_LIGHT1,GL_AMBIENT, ambient2);
    glLightfv(GL_LIGHT1,GL_DIFFUSE, diffuse2);
}

int main(int argc,char **argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(1280,720);
    glutCreateWindow("Percobaan 27");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    init();
    initHouses();

    cowX = 22.0f;
    cowZ = -35.0f;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);
    glutMouseFunc(mouseBtn);
    glutMotionFunc(mouseMove);
    glutTimerFunc(16,update,0);
    glutMainLoop();
    return 0;
}
