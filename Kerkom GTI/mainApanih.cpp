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
//Pemain
float playerX = 0.0f, playerY = 0.0f, playerZ = 12.0f;
float playerYaw = 180.0f;
float moveSpeed = 0.15f;
float turnSpeed = 3.0f;

//Lompatan
float velocityY = 0.0f;
float gravity = 0.012f;
float jumpForce = 0.22f;
float groundY = 0.0f;
bool isGrounded = true;

// --- Sistem Input ---
bool keys[256] = {false};
bool specialKeys[256] = {false};

// --- Variabel Animasi ---
float walkAnim = 0.0f;   
float swingAngle = 0.0f; 

void init()
{
    // Sky
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

void drawCylinder(float radius, float height, int slices) {
    float step = 2.0f * PI / slices;
    // Side
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
    // Top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, height, 0.0f);
    for (int i = 0; i <= slices; i++) {
        float angle = i * step;
        glVertex3f(radius * cos(angle), height, radius * sin(angle));
    }
    glEnd();
    // Bottom cap
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
    glTranslatef(playerX, playerY + 1.2f, playerZ);
    glRotatef(playerYaw, 0.0f, 1.0f, 0.0f);

    float armJumpPose = isGrounded ? 0.0f : -35.0f;

    //1. KAKI KIRI 
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

    //2. KAKI KANAN 
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

    //3. BADAN 
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

    // 4. RANSEL 
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

    //5. TANGAN KIRI
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

    //  6. TANGAN KANAN 
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

    // 7. LEHER 
    setColor(0.76f, 0.57f, 0.37f);
    glPushMatrix();
    glTranslatef(0.0f, 0.53f, 0.0f);
    glScalef(0.20f, 0.18f, 0.20f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 8. KEPALA 
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

    // 8b. RAMBUT IKAL 
    setColor(0.42f, 0.31f, 0.23f);

    // Sisi KIRI
    glPushMatrix(); glTranslatef(-0.36f, 0.98f,  0.12f); glutSolidSphere(0.150f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.38f, 0.92f, -0.05f); glutSolidSphere(0.160f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.34f, 0.88f, -0.22f); glutSolidSphere(0.145f, 12, 12); glPopMatrix();

    //  Sisi KANAN 
    glPushMatrix(); glTranslatef( 0.36f, 0.98f,  0.12f); glutSolidSphere(0.150f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.38f, 0.92f, -0.05f); glutSolidSphere(0.160f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.34f, 0.88f, -0.22f); glutSolidSphere(0.145f, 12, 12); glPopMatrix();

    //  Belakang kepala 
    glPushMatrix(); glTranslatef(-0.18f, 0.98f,  0.35f); glutSolidSphere(0.150f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.00f, 0.96f,  0.38f); glutSolidSphere(0.165f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.18f, 0.98f,  0.35f); glutSolidSphere(0.150f, 12, 12); glPopMatrix();
    // Row bawah belakang
    glPushMatrix(); glTranslatef(-0.12f, 0.87f,  0.36f); glutSolidSphere(0.140f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.12f, 0.87f,  0.36f); glutSolidSphere(0.140f, 12, 12); glPopMatrix();

    // 8c. RAMBUT BELAKANG KEPALA 
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

    // 9. RAMBUT / TOPI 
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

// =============================================================================
//  Kamera
// =============================================================================
struct Camera {
    float distance, pitch, angleAroundPlayer;
    bool dragging; int lastX, lastY;
    
    Camera() : distance(6.0f), pitch(20.0f), angleAroundPlayer(0.0f), dragging(false), lastX(0), lastY(0) {}
    
    void apply() const {
        float px = playerX;
        float py = playerY + 2.0f;
        float pz = playerZ;

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
//  Data Rumah & Pagar
// =============================================================================
struct House {
    float cx,cz,width,depth,wallH,roofH,yardD;
    int side, type; // type: 0=normal, 1=tall roof, 2=wide
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
    for(int i=0;i<count;i++) {
        for(int s=-1;s<=1;s+=2) {
            House h;
            h.side=s;
            h.type = rand() % 3; 
            
            // Variasi bentuk berdasarkan tipe
            if (h.type == 0) { // Normal
                h.width=rnd(5.0f, 6.0f); h.depth=rnd(5.5f, 7.0f);
                h.wallH=rnd(2.8f, 3.2f); h.roofH=rnd(1.8f, 2.2f);
            } else if (h.type == 1) { // Tall 
                h.width=rnd(4.5f, 5.5f); h.depth=rnd(5.0f, 6.0f);
                h.wallH=rnd(3.2f, 3.8f); h.roofH=rnd(2.5f, 3.5f);
            } else { // Wide
                h.width=rnd(7.0f, 8.5f); h.depth=rnd(5.0f, 6.5f);
                h.wallH=rnd(2.5f, 2.9f); h.roofH=rnd(1.5f, 2.0f);
            }
            
            h.yardD=rnd(1.5f,3.0f);
            h.cz=startZ - i*spacing + rnd(-0.8f,0.8f);
            float dist=roadHalf + h.yardD + h.depth*0.5f + rnd(0,0.5f);
            h.cx=s*dist;
            h.wall=wallPal[rand()%6];
            h.roof=roofPal[rand()%5];
            h.door=makeColor(0.32f,0.18f,0.08f);
            h.win=makeColor(0.60f,0.80f,0.92f);
            houses.push_back(h);
        }
    }
}

// =============================================================================
//  Objek Lingkungan (Kandang, Pohon, Rumah)
// =============================================================================
void drawFence(float x0,float z0,float x1,float z1,int posts) {
    float dx=(x1-x0)/posts, dz=(z1-z0)/posts;
    Color3 wood=makeColor(0.50f,0.34f,0.18f); float railH[2]={0.72f,0.36f};
    for(int r=0;r<2;r++){
        float th=0.035f; setColor(shade(wood, r==0?1.0f:0.85f));
        glBegin(GL_QUADS); glVertex3f(x0,railH[r]-th,z0); glVertex3f(x1,railH[r]-th,z1); glVertex3f(x1,railH[r]+th,z1); glVertex3f(x0,railH[r]+th,z0); glEnd();
    }
    for(int i=0;i<=posts;i++){
        float px=x0+dx*i, pz=z0+dz*i, lit=0.78f+0.22f*(float)(i%2);
        setColor(shade(wood,lit)); float tw=0.045f;
        glBegin(GL_QUADS); glVertex3f(px-tw,0,pz); glVertex3f(px+tw,0,pz); glVertex3f(px+tw,0.90f,pz); glVertex3f(px-tw,0.90f,pz); glEnd();
        setColor(shade(wood,lit*0.90f));
        glBegin(GL_TRIANGLES); glVertex3f(px-tw,0.90f,pz); glVertex3f(px+tw,0.90f,pz); glVertex3f(px, 1.06f,pz); glEnd();
    }
}

void drawKandangSapi(float cx, float cz) {
    float w = 4.0f, d = 4.0f;
    float x0 = cx - w/2, x1 = cx + w/2;
    float z0 = cz - d/2, z1 = cz + d/2;
    
    // Pagar keliling
    drawFence(x0, z1, x1, z1, 4); // Depan
    drawFence(x0, z0, x0, z1, 4); // Kiri
    drawFence(x1, z0, x1, z1, 4); // Kanan
    drawFence(x0, z0, x1, z0, 4); // Belakang

    // Atap peneduh sapi 
    Color3 wood = makeColor(0.40f,0.25f,0.12f);
    setColor(wood);
    // 4 Tiang
    for(int i=0; i<2; i++) {
        for(int j=0; j<2; j++) {
            float px = x0 + 0.2f + i*(w-0.4f);
            float pz = z0 + 0.2f + j*(d/2);
            glPushMatrix(); glTranslatef(px, 1.2f, pz); glScalef(0.15f, 2.4f, 0.15f); glutSolidCube(1.0f); glPopMatrix();
        }
    }
    // Atap jerami/kayu
    setColor(makeColor(0.6f, 0.5f, 0.3f));
    glPushMatrix(); glTranslatef(cx, 2.5f, z0 + d/4); glScalef(w+0.5f, 0.2f, d/2 + 0.5f); glutSolidCube(1.0f); glPopMatrix();
}

void drawKandangAyam(float cx, float cz) {
    float w = 2.0f, d = 2.0f;
    float x0 = cx - w/2, x1 = cx + w/2;
    float z0 = cz - d/2, z1 = cz + d/2;
    
    // Pagar rapat
    drawFence(x0, z1, x1, z1, 6); 
    drawFence(x0, z0, x0, z1, 6); 
    drawFence(x1, z0, x1, z1, 6); 
    drawFence(x0, z0, x1, z0, 6); 

    // Rumah ayam (Coop)
    Color3 coopColor = makeColor(0.7f, 0.3f, 0.2f);
    setColor(coopColor);
    glPushMatrix(); glTranslatef(cx, 0.5f, z0 + 0.6f); glScalef(1.2f, 1.0f, 1.0f); glutSolidCube(1.0f); glPopMatrix();
    
    // Atap coop
    setColor(makeColor(0.2f, 0.2f, 0.2f));
    glPushMatrix(); 
    glTranslatef(cx, 1.1f, z0 + 0.6f); 
    glRotatef(45, 1, 0, 0);
    glScalef(1.3f, 0.8f, 0.8f); 
    glutSolidCube(1.0f); 
    glPopMatrix();
}

static float cylX(float r,float a){ return r*cosf(a); }
static float cylZ(float r,float a){ return r*sinf(a); }

void drawTreeNatural(float tx, float tz, unsigned int seed, bool hasApples = false) {
    srand(seed);
    float trunkH  = rnd(2.5f,4.5f), rBot = 0.13f+rnd(0,0.04f), rTop = 0.05f+rnd(0,0.02f);
    Color3 bark   = makeColor(0.30f+rnd(-0.05f,0.05f), 0.20f+rnd(-0.03f,0.03f), 0.10f+rnd(-0.02f,0.02f));

    int cseg=10; float hLevels[4]={0, trunkH*0.35f, trunkH*0.72f, trunkH};
    float rLevels[4]={rBot, rBot*0.82f, rTop*1.4f, rTop};
    for(int s=0;s<3;s++) {
        float ys0=hLevels[s], ys1=hLevels[s+1], rs0=rLevels[s], rs1=rLevels[s+1];
        for(int i=0;i<cseg;i++) {
            float a0=(float)i/cseg*2*PI, a1=(float)(i+1)/cseg*2*PI;
            float lit=0.70f+0.30f*(cosf((a0+a1)*0.5f)*0.5f+0.5f);
            setColor(shade(bark,lit));
            glBegin(GL_QUADS);
                glNormal3f(cosf(a0),0.15f,sinf(a0));
                glVertex3f(tx+cylX(rs0,a0), ys0, tz+cylZ(rs0,a0)); glVertex3f(tx+cylX(rs1,a0), ys1, tz+cylZ(rs1,a0));
                glVertex3f(tx+cylX(rs1,a1), ys1, tz+cylZ(rs1,a1)); glVertex3f(tx+cylX(rs0,a1), ys0, tz+cylZ(rs0,a1));
            glEnd();
        }
    }

    Color3 leafBase = hasApples ? makeColor(0.2f, 0.45f, 0.1f) : makeColor(0.16f+rnd(-0.04f,0.04f), 0.40f+rnd(-0.06f,0.06f), 0.12f+rnd(-0.03f,0.03f));
    float crownBase = trunkH*0.38f, crownR=rnd(1.2f,2.0f); int layers=5;
    
    for(int L=0;L<layers;L++) {
        float lf=(float)L/(layers-1), layerY=crownBase + lf*(trunkH*0.65f), offX=rnd(-0.2f,0.2f);
        float layerR=crownR*(1.0f-lf*0.55f)*rnd(0.85f,1.15f), layerH=crownR*0.60f*(1.0f-lf*0.28f);
        Color3 lColor=makeColor(clampF(leafBase.r+0.04f*L,0,1), clampF(leafBase.g+0.05f*L,0,1), clampF(leafBase.b+0.02f*L,0,1));
        
        int lseg=14;
        for(int i=0;i<lseg;i++) {
            float a0=(float)i/lseg*2*PI, a1=(float)(i+1)/lseg*2*PI, am=(a0+a1)*0.5f;
            float sunLit=0.72f + 0.28f*(cosf(am)*0.55f + 0.45f);
            setColor(shade(lColor,sunLit));
            glBegin(GL_TRIANGLES);
                glNormal3f(cosf(am),0.50f,sinf(am));
                glVertex3f(tx+offX+layerR*cosf(a0), layerY, tz+layerR*sinf(a0));
                glVertex3f(tx+offX+layerR*cosf(a1), layerY, tz+layerR*sinf(a1));
                glVertex3f(tx+offX, layerY+layerH, tz);
            glEnd();

            // Apel di pinggiran daun
            if (hasApples && (rand() % 100 < 25)) {
                setColor(makeColor(0.85f, 0.1f, 0.1f));
                glPushMatrix();
                glTranslatef(tx+offX+layerR*cosf(am)*0.85f, layerY+layerH*0.3f, tz+layerR*sinf(am)*0.85f);
                glutSolidSphere(0.12f, 10, 10);
                glPopMatrix();
            }
        }
        setColor(shade(lColor,0.55f)); glBegin(GL_TRIANGLE_FAN); glNormal3f(0,-1,0); glVertex3f(tx+offX,layerY,tz);
        for(int i=0;i<=lseg;i++){ float a=(float)i/lseg*2*PI; glVertex3f(tx+offX+layerR*cosf(a),layerY,tz+layerR*sinf(a)); } glEnd();
    }
}

void drawWindow(float wx0,float wy0,float wz,float wx1,float wy1, Color3 winC, float normalZ) {
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

void drawHouse(const House &h) {
    float x0=h.cx-h.width*0.5f, x1=h.cx+h.width*0.5f;
    float z0=h.cz-h.depth*0.5f, z1=h.cz+h.depth*0.5f;
    float y0=0, y1=h.wallH, ov=0.35f;

    Color3 wF=h.wall, wS=shade(h.wall,0.80f), wB=shade(h.wall,0.65f);
    setColor(wF); quad4(x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y1,z1);
    setColor(wB); quad4(x1,y0,z0, x0,y0,z0, x0,y1,z0, x1,y1,z0);
    setColor(wS); quad4(x0,y0,z0, x0,y0,z1, x0,y1,z1, x0,y1,z0);
    setColor(shade(h.wall,0.87f)); quad4(x1,y0,z1, x1,y0,z0, x1,y1,z0, x1,y1,z1);

    {
        float dw=h.width*0.16f, dh=h.wallH*0.52f, dx0=h.cx-dw*0.5f, dx1=h.cx+dw*0.5f;
        setColor(h.door); glNormal3f(0,0,1);
        glBegin(GL_QUADS);
            glVertex3f(dx0,y0,z1+0.01f); glVertex3f(dx1,y0,z1+0.01f); glVertex3f(dx1,dh,z1+0.01f); glVertex3f(dx0,dh,z1+0.01f);
        glEnd();
    }
    {
        float ww=h.width*0.15f, wh=h.wallH*0.27f, wb=y0+h.wallH*0.50f, wt=wb+wh;
        float pxArr[2]={h.cx-h.width*0.28f, h.cx+h.width*0.28f};
        for(int i=0;i<2;i++) {
            drawWindow(pxArr[i]-ww*0.5f, wb, z1+0.01f, pxArr[i]+ww*0.5f, wt, h.win, 1.0f);
        }
    }

    float rx0=x0-ov, rx1=x1+ov, rz0=z0-ov, rz1=z1+ov, rY=y1+h.roofH, rX=h.cx;
    setColor(h.roof); quad4(rx0,y1,rz1, rx0,y1,rz0, rX,rY,rz0, rX,rY,rz1);
    setColor(shade(h.roof,0.68f)); quad4(rx1,y1,rz0, rx1,y1,rz1, rX,rY,rz1, rX,rY,rz0);
    setColor(shade(h.wall,0.92f)); tri3(rx0,y1,rz1, rx1,y1,rz1, rX,rY,rz1);
    setColor(shade(h.wall,0.72f)); tri3(rx1,y1,rz0, rx0,y1,rz0, rX,rY,rz0);
}

// =============================================================================
//  Jalan & Tanah
// =============================================================================
void drawRoad() {
    float rw=6.0f, zN=20.0f, zF=-150.0f, sw=1.4f;
    glColor3f(0.20f,0.20f,0.20f); glNormal3f(0,1,0);
    glBegin(GL_QUADS); glVertex3f(-rw*.5f,0.005f,zN); glVertex3f(rw*.5f,0.005f,zN); glVertex3f(rw*.5f,0.005f,zF);  glVertex3f(-rw*.5f,0.005f,zF); glEnd();
    glColor3f(0.40f,0.34f,0.25f); glNormal3f(0,1,0);
    for(int s=-1;s<=1;s+=2){
        float sx0=s*rw*.5f, sx1=s*(rw*.5f+sw);
        glBegin(GL_QUADS); glVertex3f(sx0,0.003f,zN); glVertex3f(sx1,0.003f,zN); glVertex3f(sx1,0.003f,zF); glVertex3f(sx0,0.003f,zF); glEnd();
    }
}

void drawGround() {
    glColor3f(0.30f,0.25f,0.18f); glNormal3f(0,1,0);
    glBegin(GL_QUADS); glVertex3f(-300,0,25); glVertex3f(300,0,25); glVertex3f(300,0,-200); glVertex3f(-300,0,-200); glEnd();
    glColor3f(0.24f,0.38f,0.16f); glNormal3f(0,1,0);
    glBegin(GL_QUADS); glVertex3f(-100,0.002f,22); glVertex3f(100,0.002f,22); glVertex3f(100,0.002f,-180); glVertex3f(-100,0.002f,-180); glEnd();
}

void drawSky() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    float zS=-180.0f, xW=300.0f;
    struct SkyBand { float y0,y1; float r0,g0,b0,r1,g1,b1; };
    SkyBand bands[]={
        {0,  8,   0.72f, 0.62f, 0.55f,  0.65f, 0.55f, 0.60f},
        {8,  20,  0.65f, 0.55f, 0.60f,  0.45f, 0.50f, 0.70f},
        {20, 45,  0.45f, 0.50f, 0.70f,  0.25f, 0.35f, 0.65f},
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
void drawScene() {
    drawSky();
    drawGround();
    drawRoad();

    // Gambar Rumah
    for(int i=0;i<(int)houses.size();i++)
        drawHouse(houses[i]);

    srand(123);

    // Pohon Belakang Rumah
    for(int i=0;i<(int)houses.size();i++){
        const House &h=houses[i];
        float tz = h.cz - (h.depth * 0.5f) - rnd(3.0f, 5.0f);
        float tx = h.cx + rnd(-h.width*0.4f, h.width*0.4f);
        drawTreeNatural(tx, tz, (unsigned)(i*9+3));
    }

    // Kandang Sapi
    if(houses.size() > 2) {
        drawKandangSapi(houses[2].cx, houses[2].cz - houses[2].depth - 6.0f);
    }
    // Kandang Ayam
    if(houses.size() > 6) {
        drawKandangAyam(houses[6].cx, houses[6].cz - houses[6].depth - 4.0f);
    }

    // Perkebunan Apel
    for(int row=0; row<4; row++) {
        for(int col=0; col<5; col++) {
            float ax = -15.0f - col * 5.0f + rnd(-0.5f, 0.5f);
            float az = -20.0f - row * 6.0f + rnd(-0.5f, 0.5f);
            drawTreeNatural(ax, az, (unsigned)(row*10+col), true);
        }
    }

    // Pagar keliling rumah
    for(int i=0;i<(int)houses.size();i+=2){
        const House &h=houses[i];
        drawFence(h.cx-h.width*0.5f, h.cz+h.depth*0.5f+0.08f,
                  h.cx+h.width*0.5f, h.cz+h.depth*0.5f+0.08f, 8);
    }
    
    drawPlayer();
}

// =============================================================================
//  Lingkungan & Main
// =============================================================================
void setupEnvironment() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    GLfloat sunPos[]={-15, 12, 10, 0};
    GLfloat sunAmb[]={0.25f, 0.20f, 0.15f, 1};
    GLfloat sunDif[]={1.00f, 0.85f, 0.65f, 1}; 
    GLfloat sunSpc[]={0.30f, 0.25f, 0.20f, 1};
    glLightfv(GL_LIGHT0,GL_POSITION,sunPos);
    glLightfv(GL_LIGHT0,GL_AMBIENT, sunAmb);
    glLightfv(GL_LIGHT0,GL_DIFFUSE, sunDif);
    glLightfv(GL_LIGHT0,GL_SPECULAR,sunSpc);

    GLfloat skyPos[]={10, 20, -10, 0};
    GLfloat skyDif[]={0.20f, 0.25f, 0.35f, 1}; 
    GLfloat skyAmb[]={0.10f, 0.15f, 0.20f, 1};
    GLfloat noSpec[]={0,0,0,1};
    glLightfv(GL_LIGHT1,GL_POSITION,skyPos);
    glLightfv(GL_LIGHT1,GL_DIFFUSE, skyDif);
    glLightfv(GL_LIGHT1,GL_AMBIENT, skyAmb);
    glLightfv(GL_LIGHT1,GL_SPECULAR,noSpec);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat matSpc[]={0.05f,0.05f,0.05f,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,matSpc);
    glMaterialf (GL_FRONT_AND_BACK,GL_SHININESS,5.0f);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP2);
    GLfloat fogColor[] = {0.72f, 0.62f, 0.55f, 1.0f}; 
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.012f); 
    glHint(GL_FOG_HINT, GL_NICEST);
}

void display() {
    glClearColor(0.72f, 0.62f, 0.55f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    cam.apply();
    setupEnvironment();
    drawScene();
    glutSwapBuffers();
}

void reshape(int w,int h) {
	if (h == 0) h = 1;
    float ratio = (float)w / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, ratio, 0.1, 100);
    glViewport(0, 0, w, h);
    
    if(!h)h=1;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(50.0f,(float)w/h,0.05f,400.0f);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int, int) {
    keys[key] = true;
    if (key == 27) exit(0);
    if (key == 'r' || key == 'R') {
        playerX = 0.0f; playerY = groundY; playerZ = 12.0f;
        playerYaw = 180.0f;
        velocityY = 0.0f; isGrounded = true;
        cam.angleAroundPlayer = 0.0f; cam.pitch = 20.0f; cam.distance = 6.0f;
    }
}

void keyboardUp(unsigned char key, int, int) {
    keys[key] = false;
}

void specialKey(int key, int, int) {
    specialKeys[key] = true;
}

void specialKeyUp(int key, int, int) {
    specialKeys[key] = false;
}

void mouseBtn(int btn,int state,int x,int y){
    if(btn==GLUT_LEFT_BUTTON){cam.dragging=(state==GLUT_DOWN);cam.lastX=x;cam.lastY=y;}
    if(btn==3){cam.distance -= 0.5f; glutPostRedisplay();} 
    if(btn==4){cam.distance += 0.5f; glutPostRedisplay();} 
    if(cam.distance < 1.5f) cam.distance = 1.5f;
}

void mouseMove(int x,int y){
    if(!cam.dragging)return;
    int dx=x-cam.lastX, dy=y-cam.lastY;
    cam.lastX=x; cam.lastY=y;
    
    cam.angleAroundPlayer -= dx*0.4f;
    cam.pitch -= dy*0.4f;
    
    if(cam.pitch> 80.0f)cam.pitch= 80.0f; 
    if(cam.pitch<-10.0f)cam.pitch=-10.0f; 
    glutPostRedisplay();
}

void update(int value)
{
    float rad = playerYaw * (PI / 180.0f);
    bool isWalking = false;

    // WASD + Arrow Keys untuk gerak
    bool moveForward  = keys['w'] || keys['W'] || specialKeys[GLUT_KEY_UP];
    bool moveBackward = keys['s'] || keys['S'] || specialKeys[GLUT_KEY_DOWN];
    bool turnLeft     = keys['a'] || keys['A'] || specialKeys[GLUT_KEY_LEFT];
    bool turnRight    = keys['d'] || keys['D'] || specialKeys[GLUT_KEY_RIGHT];

    if (moveForward) {
        playerX -= sinf(rad) * moveSpeed;
        playerZ -= cosf(rad) * moveSpeed;
        isWalking = true;
    }
    if (moveBackward) {
        playerX += sinf(rad) * moveSpeed;
        playerZ += cosf(rad) * moveSpeed;
        isWalking = true;
    }
    if (turnLeft)  playerYaw += turnSpeed;
    if (turnRight) playerYaw -= turnSpeed;

    // Space = Lompat
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
            swingAngle = sinf(walkAnim) * 42.0f;
        } else {
            if (swingAngle > 0) swingAngle -= 2.0f;
            if (swingAngle < 0) swingAngle += 2.0f;
            if (fabsf(swingAngle) < 2.0f) swingAngle = 0.0f;
        }
    } else {
        swingAngle = 0.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc,char **argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(1280,720);
    glutCreateWindow("Percobaan 12");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    initHouses();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);
    glutMouseFunc(mouseBtn);
    glutMotionFunc(mouseMove);
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}
