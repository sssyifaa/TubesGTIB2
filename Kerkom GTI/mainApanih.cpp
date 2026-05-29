#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <cstdio>

static const float PI = 3.14159265f;
static const float D2R = PI / 180.0f;


//  Utilitas

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


//  Tekstur Prosedural

static GLuint texGrass = 0;
static GLuint texDirt  = 0;
static GLuint texRoad  = 0;
static GLuint texWood  = 0;

static float smoothNoise(int x, int y, int seed) {
    int n = x * 1619 + y * 31337 + seed * 3791;
    n = (n << 13) ^ n;
    return 1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
}
static GLuint makeTexture(unsigned char* data, int w, int h) {
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);
    return id;
}
static unsigned char clampByte(int v) {
    return (unsigned char)(v < 0 ? 0 : v > 255 ? 255 : v);
}
static void generateTextures() {
    const int SZ = 128;
    unsigned char buf[SZ * SZ * 3];
    // RUMPUT
    for(int y=0;y<SZ;y++) for(int x=0;x<SZ;x++) {
        float n=(smoothNoise(x,y,1)*0.5f+smoothNoise(x/2,y/2,2)*0.3f+smoothNoise(x/4,y/4,3)*0.2f)*0.5f+0.5f;
        int idx=(y*SZ+x)*3;
        buf[idx+0]=clampByte((int)(58+n*18)+((x*7+y*13)%40==0?5:0));
        buf[idx+1]=clampByte((int)(130+n*25)+((x*7+y*13)%40==0?20:0));
        buf[idx+2]=clampByte((int)(45+n*12));
    }
    texGrass=makeTexture(buf,SZ,SZ);
    // TANAH
    for(int y=0;y<SZ;y++) for(int x=0;x<SZ;x++) {
        float n=(smoothNoise(x,y,10)*0.5f+smoothNoise(x/2,y/2,11)*0.3f+smoothNoise(x/4,y/4,12)*0.2f)*0.5f+0.5f;
        int idx=(y*SZ+x)*3;
        buf[idx+0]=clampByte((int)(120+n*30));
        buf[idx+1]=clampByte((int)(88+n*20));
        buf[idx+2]=clampByte((int)(55+n*12));
    }
    texDirt=makeTexture(buf,SZ,SZ);
    // ASPAL
    for(int y=0;y<SZ;y++) for(int x=0;x<SZ;x++) {
        float n=(smoothNoise(x,y,20)*0.4f+smoothNoise(x/2,y/2,21)*0.3f)*0.5f+0.5f;
        int v=(int)(55+n*20);
        if(x>=60&&x<=68&&(y/16)%2==0) v=210;
        int idx=(y*SZ+x)*3;
        buf[idx+0]=buf[idx+1]=buf[idx+2]=clampByte(v);
    }
    texRoad=makeTexture(buf,SZ,SZ);
    // KAYU
    for(int y=0;y<SZ;y++) for(int x=0;x<SZ;x++) {
        int plank=y/16;
        float plankPos=(float)(y%16)/16.0f;
        float grain=(smoothNoise(x/3,plank,30)*0.4f+smoothNoise(x/6,plank,31)*0.3f)*0.5f+0.5f;
        float edge=0.75f+(1.0f-2.0f*fabsf(plankPos-0.5f))*0.25f;
        float base=(plank%2==0)?0.85f:0.78f;
        int r=(int)(160*base*edge*grain),g=(int)(105*base*edge*grain),b=(int)(55*base*edge*grain);
        if(y%16==0||y%16==15){r/=2;g/=2;b/=2;}
        int idx=(y*SZ+x)*3;
        buf[idx+0]=clampByte(r);buf[idx+1]=clampByte(g);buf[idx+2]=clampByte(b);
    }
    texWood=makeTexture(buf,SZ,SZ);
}
static void enableTex(GLuint id) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
static void disableTex() { glDisable(GL_TEXTURE_2D); }


//  Player (Karakter)

//float playerX = 0.0f, playerY = 0.0f, playerZ = 12.0f;
float playerX = 0.0f, playerY = 0.0f, playerZ = -95.0f; //tar ganti
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


//  Collision System (AABB)

struct AABB { float minX, maxX, minZ, maxZ; };
std::vector<AABB> colliders;
const float PLAYER_RADIUS = 0.4f;

struct TreePos { float x, z; };
std::vector<TreePos> treesApple; 

static void addBox(float cx, float cz, float w, float d){
    AABB b;
    b.minX=cx-w*0.5f; b.maxX=cx+w*0.5f;
    b.minZ=cz-d*0.5f; b.maxZ=cz+d*0.5f;
    colliders.push_back(b);
}
static bool checkCollision(float nx, float nz){
    for(int i=0;i<(int)colliders.size();i++){
        if(nx+PLAYER_RADIUS > colliders[i].minX &&
           nx-PLAYER_RADIUS < colliders[i].maxX &&
           nz+PLAYER_RADIUS > colliders[i].minZ &&
           nz-PLAYER_RADIUS < colliders[i].maxZ)
            return true;
    }
    return false;
}


// Tisya NPC 

float tisyaIdleTime = 0.0f;
float tisyaX = 0.0f;
float tisyaZ = 9.0f;
float tisyaYaw = 180.0f; // arah hadap
static bool  showLevelUpBanner = false;
static float levelUpBannerTimer = 0.0f;
static bool  showRumahBanner     = false;
static float rumahBannerTimer    = 0.0f;
static bool  nearRumahBro        = false;
static bool  insideRumahBro      = false;
static float prevPlayerX         = 0.0f;
static float prevPlayerZ         = 0.0f;
static const float RUMAH_INTERIOR_X  = 500.0f;
static const float RUMAH_INTERIOR_Z  = 500.0f;
static const float RUMAH_DOOR_Z_OUT  = 507.5f;
static const float RUMAH_INTERACT_RANGE = 5.5f;
static const float RUMAH_DOOR_RANGE     = 2.0f;


//  State Sapi

static float cowX      = 22.f, cowZ = -35.f;
static float cowAngle  = 0.f;
static float walkPhase = 0.f;
static float tailPhase = 0.f;
static float udderSwing= 0.f;
static int   milkCount = 0;
static int   isMillking = 0;
static float milkAnim  = 0.f;

static const float COW_KANDANG_CX = 22.0f;
static const float COW_KANDANG_CZ = -35.0f;
static const float COW_KANDANG_W  = 8.0f;
static const float COW_KANDANG_D  = 8.0f;
static const float COW_MARGIN     = 1.4f;

struct Waypoint { float x, z; };
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
static bool  cowIsMoving        = true;

static const float MILK_INTERACT_RANGE = 3.0f;
static bool  nearCow    = false;
static bool  milkPrompt = false;


//  Gerbang State

static const float GATE_INTERACT_RANGE = 2.5f;

// Gerbang kandang sapi
static const float GATE_SAPI_X    = 22.0f;
static const float GATE_SAPI_Z    = -31.0f;  
static bool  gateSapiOpen         = false;
static float gateSapiAngle        = 0.0f;    
static bool  nearGateSapi         = false;

// Gerbang kandang ayam
static const float GATE_AYAM_X    = 32.0f;
static const float GATE_AYAM_Z    = -31.0f;
static bool  gateAyamOpen         = false;
static float gateAyamAngle        = 0.0f;
static bool  nearGateAyam         = false;

static float TONG_X = 0.f;
static float TONG_Z = 0.f;
static const float MILK_RANGE = 3.5f;


//  State Ayam - 5 ekor dengan gerakan acak

static const int NUM_CHICKENS = 5;
static const float CHICKEN_CX = 32.0f;
static const float CHICKEN_CZ = -35.0f;
static const float CHICKEN_LIMIT_W = 8.0f;
static const float CHICKEN_LIMIT_D = 8.0f;
static const float CH_XMIN = CHICKEN_CX - CHICKEN_LIMIT_W*0.5f + 0.4f;
static const float CH_XMAX = CHICKEN_CX + CHICKEN_LIMIT_W*0.5f - 0.4f;
static const float CH_ZMIN = CHICKEN_CZ - CHICKEN_LIMIT_D*0.5f + 0.4f;
static const float CH_ZMAX = CHICKEN_CZ + CHICKEN_LIMIT_D*0.5f - 0.4f;

struct Chicken {
    float x, z;
    float angle;
    float legAnim;
    float speed;
    int   turnTimer;
    float turnTarget;
};

static Chicken chickens[NUM_CHICKENS];


//  MISSION SYSTEM

enum GameState {
    GS_MENU,
    GS_STORY,
    GS_PLAYING,
    GS_MISSION_SUCCESS,
    GS_MISSION_FAIL,
    GS_LEVEL_UP,
    GS_GAME_OVER,
    GS_WIN
};

enum MissionID {
    MISSION_NONE = 0,
    MISSION_APEL,       // Level 1
    MISSION_TELUR,      // Level 2
    MISSION_SUSU        // Level 3
};

enum RelationLevel {
    REL_STRANGER = 0,
    REL_PACARAN,
    REL_PERTUNANGAN,
    REL_PERNIKAHAN
};

static GameState  gameState      = GS_STORY;
static MissionID  currentMission = MISSION_APEL;
static RelationLevel relationLevel = REL_STRANGER;

static int playerLives  = 3;
static float missionTimer = 60.0f;  
static bool missionActive = false;

// Apel (Level 1) 
static const int TOTAL_APPLES = 10;
static int applesCollected   = 0;

struct AppleItem {
    float x, y, z;
    bool collected;
};
static AppleItem appleItems[TOTAL_APPLES];
static bool applesInitialized = false;
static const float APPLE_COLLECT_RANGE = 1.8f;

// Telur Ayam (Level 2)
static const int TOTAL_EGGS = 8;
static int eggsCollected = 0;
struct EggItem { float x, z; bool collected; };
static EggItem eggItems[TOTAL_EGGS];
static bool eggsInitialized = false;
static const float EGG_COLLECT_RANGE = 1.5f;

// Susu Sapi (Level 3)
static const int TARGET_MILK = 5;

// Dialog System
static const int MAX_DIALOG_LINES = 6;
struct DialogLine {
    const char* speaker; // "Narator", "Tisya", "Gilang"
    const char* text;
};
static DialogLine dialogLines[MAX_DIALOG_LINES];
static int dialogCount     = 0;
static int dialogIndex     = 0;
static bool dialogActive   = false;
static bool nearTisya      = false;
static const float TISYA_TALK_RANGE = 1.5f;

// Teks dialog untuk tiap state 
static void pushDialog(const char* spk, const char* txt) {
    if(dialogCount < MAX_DIALOG_LINES) {
        dialogLines[dialogCount].speaker = spk;
        dialogLines[dialogCount].text    = txt;
        dialogCount++;
    }
}
static void clearDialog() { dialogCount = 0; dialogIndex = 0; dialogActive = false; }

static void showStoryBefore() {
    clearDialog();
    if(currentMission == MISSION_APEL) {
	    pushDialog("Gilang",  "Hai, kamu Tisya kan? Aku Gilang.");
	    pushDialog("Tisya",   "Iya, ada apa ya?");
	    pushDialog("Gilang",  "Aku... suka sama kamu. Boleh aku buktiin perasaanku?");
	    pushDialog("Tisya",   "Hmm, kalau kamu bisa kumpulkan apel-apel itu untukku, aku mau dengarkan.");
	    pushDialog("Narator", "Kumpulkan semua apel sebelum waktu habis!");
    } else if(currentMission == MISSION_TELUR) {
        pushDialog("Narator", "Setelah berpacaran, Gilang mengantarkan Tisya pulang...");
        pushDialog("Tisya",   "Kalau jadi pasangan, harus bisa sabar dan sigap, Gilang.");
        pushDialog("Gilang",  "Aku sigap! Apa yang kamu butuhin?");
        pushDialog("Tisya",   "Kasih aku sesuatu yang membuktikan effort dan usaha kamu.");
        pushDialog("Narator", "Cari kandang ayam di desa ini dan ambil telur dari ayam yang terus bergerak!");
    } else if(currentMission == MISSION_SUSU) {
        pushDialog("Gilang",  "Tisya, aku mau lanjut ke jenjang yang lebih serius dengan kamu.");
        pushDialog("Tisya",   "Kamu harus menunjukan usaha kamu untuk sampai ke titik itu.");
        pushDialog("Gilang",  "Aku siap!");
        pushDialog("Narator", "Pergi ke peternakan dan perah susu dari sapi yang terus bergerak, sebanyak 5 liter!");
    }
    dialogActive = true;
    gameState = GS_STORY;
}

static void showSuccessDialog() {
    clearDialog();
    if(currentMission == MISSION_APEL) {
        pushDialog("Tisya",  "Kamu benar-benar berusaha ya...");
        pushDialog("Gilang", "Karena aku nggak mau kehilangan kesempatan buat dekat sama kamu.");
        pushDialog("Tisya",  "Kalau begitu... mulai sekarang kita pacaran.");
    } else if(currentMission == MISSION_TELUR) {
        pushDialog("Tisya",  "Kamu sedang apa Gilang?");
        pushDialog("Gilang", "Aku mau serius dengan kamu Tisya, ayo bertunangan..");
        pushDialog("Tisya",  "Aku mau...");
    } else if(currentMission == MISSION_SUSU) {
        pushDialog("Gilang", "Tisya, ini kue untuk mu, bukti keseriusanku kepadamu.");
        pushDialog("Tisya",  "Terimakasih, Gilang. Kamu sudah membuktikan keseriusanmu.");
        pushDialog("Tisya",  "Ayo kita lanjut ke hari paling bahagia.");
    }
    dialogActive = true;
    gameState = GS_MISSION_SUCCESS;
    missionActive = false;
}

static void showFailDialog() {
    clearDialog();
    if(currentMission == MISSION_APEL) {
        pushDialog("Tisya",  "Yah... padahal tinggal sedikit lagi.");
        pushDialog("Gilang", "Aku gagal...");
        pushDialog("Tisya",  "Mungkin... perasaan kamu nggak sebesar yang kamu bilang.");
        pushDialog("Gilang", "Bukan gitu, Tisya, aku---");
    } else if(currentMission == MISSION_TELUR) {
        pushDialog("Gilang", "Kenapa sulit sekali...");
        pushDialog("Tisya",  "Mana bukti dari usaha kamu untukku?");
        pushDialog("Gilang", "Maaf Tisya...");
    } else if(currentMission == MISSION_SUSU) {
        pushDialog("Gilang", "Aku belum berhasil mengumpulkan cukup susu...");
        pushDialog("Tisya",  "Kamu perlu lebih bersungguh-sungguh, Gilang.");
    }
    dialogActive = true;
    gameState = GS_MISSION_FAIL;
}

static void showRetryDialog() {
    clearDialog();
    if(currentMission == MISSION_APEL) {
        pushDialog("Tisya",   "Coba lagi ya, masih ada kesempatan.");
        pushDialog("Narator", "Kumpulkan semua apel sebelum waktu habis!");
    } else if(currentMission == MISSION_TELUR) {
        pushDialog("Tisya",   "Jangan menyerah, Gilang.");
        pushDialog("Narator", "Ambil semua telur dari kandang ayam!");
    } else if(currentMission == MISSION_SUSU) {
        pushDialog("Tisya",   "Kamu harus lebih bersungguh-sungguh.");
        pushDialog("Narator", "Perah susu sapi sebanyak 5 liter!");
    }
    dialogActive = true;
    gameState = GS_STORY;
}

// Timer counter 
static int timerTicks = 0;

static void initAppleItems() {
    float positions[TOTAL_APPLES][2] = {
        {-15.5f, -22.0f}, {-20.0f, -24.0f}, {-25.5f, -26.0f},
        {-15.0f, -28.0f}, {-30.0f, -22.0f}, {-35.0f, -26.0f},
        {-20.5f, -32.0f}, {-25.0f, -34.0f}, {-30.5f, -32.0f},
        {-35.0f, -34.0f}
    };
    for(int i = 0; i < TOTAL_APPLES; i++) {
        appleItems[i].x = positions[i][0];
        appleItems[i].y = 1.2f;
        appleItems[i].z = positions[i][1];
        appleItems[i].collected = false;
    }
    applesInitialized = true;
}

static void initEggItems() {
    float cx = CHICKEN_CX, cz = CHICKEN_CZ;
    float offX[TOTAL_EGGS] = {-2.5f, 2.5f, -1.0f, 1.0f, -2.5f, 2.5f, 0.0f, -1.5f};
    float offZ[TOTAL_EGGS] = {-2.5f, -2.0f, -1.0f, 2.0f, 2.5f, 2.5f, -2.0f, 1.5f};
    for(int i = 0; i < TOTAL_EGGS; i++) {
        eggItems[i].x = cx + offX[i];
        eggItems[i].z = cz + offZ[i];
        eggItems[i].collected = false;
    }
    eggsInitialized = true;
}

// apel di tanah
static void drawGroundApple(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    setColor(0.85f, 0.10f, 0.10f);
    glutSolidSphere(0.18f, 8, 8);
    setColor(0.20f, 0.55f, 0.10f);
    glTranslatef(0, 0.18f, 0);
    glScalef(0.04f, 0.12f, 0.04f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// telur
static void drawEgg(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.12f, z);
    setColor(0.95f, 0.92f, 0.85f);
    glScalef(0.8f, 1.0f, 0.8f);
    glutSolidSphere(0.12f, 8, 8);
    glPopMatrix();
}

// Hati animasi
static float heartAnim = 0.0f;

void initChickens() {
    float startPos[NUM_CHICKENS][2] = {
        {32.0f, -35.0f},
        {30.5f, -33.5f},
        {33.5f, -33.5f},
        {30.5f, -36.5f},
        {33.5f, -36.5f},
    };
    for(int i = 0; i < NUM_CHICKENS; i++) {
        chickens[i].x        = startPos[i][0];
        chickens[i].z        = startPos[i][1];
        chickens[i].angle    = (float)(i * 72);
        chickens[i].legAnim  = (float)i * 0.5f;
        chickens[i].speed    = 0.07f + 0.03f * i;
        chickens[i].turnTimer = 30 + i * 20;
        chickens[i].turnTarget = chickens[i].angle;
    }
}


//  Kamera

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


//  Data Rumah

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
    // Rumah Bro paling ujung
	{
	    House h;
	    h.side  = 1;
	    h.type  = 1;
	    h.width = 5.0f; h.depth = 6.0f;
	    h.wallH = 3.5f; h.roofH = 3.0f;
	    h.yardD = 2.0f;
	    h.cz    = -106.0f;
	    h.cx    = 8.0f;
	    h.wall  = wallPal[1];
	    h.roof  = roofPal[2];
	    h.door  = makeColor(0.32f, 0.18f, 0.08f);
	    h.win   = makeColor(0.60f, 0.80f, 0.92f);
	    houses.push_back(h);
	}
}


//  Init Colliders 

void initColliders(){
    colliders.clear();

    for(int i=0;i<(int)houses.size();i++){
        const House &h=houses[i];
        addBox(h.cx, h.cz, h.width+0.3f, h.depth+0.3f);
    }

    // PAGAR + SAPI kandang sapi 
    addBox(22.0f, -39.0f, 8.5f, 0.4f); 
	addBox(18.0f, -35.0f, 0.4f, 8.5f);
	addBox(26.0f, -35.0f, 0.4f, 8.5f);
	addBox(22.0f, -35.0f, 2.0f, 3.0f);
	addBox(19.5f, -31.0f, 3.5f, 0.4f);
	addBox(24.5f, -31.0f, 3.5f, 0.4f);
	if(!gateSapiOpen) addBox(22.0f, -31.0f, 2.0f, 0.4f);

    // PAGAR kandang ayam 
    addBox(32.0f, -39.0f, 8.5f, 0.4f);  
	addBox(28.0f, -35.0f, 0.4f, 8.5f);  
	addBox(36.0f, -35.0f, 0.4f, 8.5f);  
	addBox(29.5f, -31.0f, 3.5f, 0.4f); 
	addBox(34.5f, -31.0f, 3.5f, 0.4f); 
	if(!gateAyamOpen) addBox(32.0f, -31.0f, 2.0f, 0.4f);  

    // POHON APEL 
    for(int i=0;i<(int)treesApple.size();i++)
        addBox(treesApple[i].x, treesApple[i].z, 0.9f, 0.9f);

    // TISYA 
    addBox(tisyaX, tisyaZ, 1.0f, 1.0f);
    
    // rumah bro
    if(insideRumahBro) {
        float rx = RUMAH_INTERIOR_X, rz = RUMAH_INTERIOR_Z;
        float RW = 20.0f, RD = 20.0f;
        addBox(rx,           rz - RD/2,  RW,     0.5f); 
        addBox(rx,           rz + RD/2,  RW,     0.5f); 
        addBox(rx - RW/2,    rz,         0.5f,   RD);  
        addBox(rx + RW/2,    rz,         0.5f,   RD);
    }
}


//  Gambar Pagar

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


//  Kandang Sapi

// gerbang
void drawGate(float hinge_x, float hinge_z, float gateAngleDeg, float gateWidth, bool isOpenLeft) {
    Color3 wood     = makeColor(0.45f, 0.28f, 0.12f);
    Color3 woodDark = makeColor(0.32f, 0.20f, 0.08f);

    // Tiang engsel
    setColor(woodDark);
    glPushMatrix();
    glTranslatef(hinge_x - 0.12f, 0, hinge_z);
    glScalef(0.24f, 1.5f, 0.24f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Tiang kanan
    glPushMatrix();
    glTranslatef(hinge_x + gateWidth + 0.12f, 0, hinge_z);
    glScalef(0.24f, 1.5f, 0.24f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Daun gerbang 
    glPushMatrix();
    glTranslatef(hinge_x, 0, hinge_z);
    float rotDir = isOpenLeft ? 1.0f : -1.0f;
    glRotatef(rotDir * gateAngleDeg, 0, 1, 0);

    // Rail atas
    setColor(wood);
    glPushMatrix();
    glTranslatef(gateWidth*0.5f, 1.1f, 0);
    glScalef(gateWidth, 0.10f, 0.10f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Rail bawah
    glPushMatrix();
    glTranslatef(gateWidth*0.5f, 0.35f, 0);
    glScalef(gateWidth, 0.10f, 0.10f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Rail tengah
    glPushMatrix();
    glTranslatef(gateWidth*0.5f, 0.72f, 0);
    glScalef(gateWidth, 0.08f, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Papan vertikal (4 buah)
    setColor(shade(wood, 0.9f));
    int boards = 4;
    for(int i = 0; i < boards; i++) {
        float bx = gateWidth * (i + 0.5f) / boards;
        glPushMatrix();
        glTranslatef(bx, 0.72f, 0);
        glScalef(0.10f, 1.44f, 0.08f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Diagonal gerbang
    setColor(shade(wood, 0.85f));
    glPushMatrix();
    glTranslatef(gateWidth*0.5f, 0.72f, 0);
    float diagAngle = atan2f(0.75f, gateWidth) * 180.0f / PI;
    glRotatef(diagAngle, 0, 0, 1);
    glScalef(sqrtf(gateWidth*gateWidth + 0.75f*0.75f), 0.07f, 0.07f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Diagonal gerbang
    glPushMatrix();
    glTranslatef(gateWidth*0.5f, 0.72f, 0);
    glRotatef(-diagAngle, 0, 0, 1);
    glScalef(sqrtf(gateWidth*gateWidth + 0.75f*0.75f), 0.07f, 0.07f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix(); 
}

void drawKandangSapi(float cx, float cz) {
    float w=8.0f, d=8.0f;
    float x0=cx-w/2, x1=cx+w/2;
    float z0=cz-d/2, z1=cz+d/2;
    float gateW = 2.0f; 
    float gateCenter = cx;

    drawFence(x0,z0,x1,z0,8);       
	drawFence(x0,z0,x0,z1,8);        
	drawFence(x1,z0,x1,z1,8);        
	drawFence(x0, z1, gateCenter - gateW*0.5f, z1, 3);
	drawFence(gateCenter + gateW*0.5f, z1, x1, z1, 3);
	drawGate(gateCenter - gateW*0.5f, z1, gateSapiAngle, gateW, false);

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


//  Kandang Ayam

void drawKandangAyam(float cx, float cz) {
    float w=8.0f, d=8.0f;  
    float x0=cx-w/2, x1=cx+w/2;
    float z0=cz-d/2, z1=cz+d/2;
    float gateW = 2.0f;
    float gateCenter = cx;

    drawFence(x0,z0,x1,z0,8);
	drawFence(x0,z0,x0,z1,8);
	drawFence(x1,z0,x1,z1,8);
	drawFence(x0, z1, gateCenter - gateW*0.5f, z1, 3);
	drawFence(gateCenter + gateW*0.5f, z1, x1, z1, 3);
	drawGate(gateCenter - gateW*0.5f, z1, gateAyamAngle, gateW, false);

    // Lantai kandang
    setColor(0.35f, 0.45f, 0.1f);
    glBegin(GL_QUADS);
        glVertex3f(x0, 0.001f, z0);
        glVertex3f(x1, 0.001f, z0);
        glVertex3f(x1, 0.001f, z1);
        glVertex3f(x0, 0.001f, z1);
    glEnd();

    setColor(0.7f,0.3f,0.2f);
    glPushMatrix(); glTranslatef(cx,0.5f,z0+0.6f); glScalef(1.2f,1.0f,1.0f); glutSolidCube(1.0f); glPopMatrix();
    setColor(0.2f,0.2f,0.2f);
    glPushMatrix(); glTranslatef(cx,1.1f,z0+0.6f); glRotatef(45,1,0,0); glScalef(1.3f,0.8f,0.8f); glutSolidCube(1.0f); glPopMatrix();
}


//  Gambar Sapi

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


//  Player

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


//  Tisya 

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

    // Badan 
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

    // Rambut bondol 
    setColor(0.16f, 0.15f, 0.15f);
    glPushMatrix(); glTranslatef(0.0f, 1.06f, 0.02f); glScalef(0.60f, 0.18f, 0.56f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.32f, 0.84f, 0.00f); glScalef(0.10f, 0.44f, 0.50f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.32f, 0.84f, 0.00f); glScalef(0.10f, 0.44f, 0.50f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.84f, 0.30f); glScalef(0.56f, 0.46f, 0.10f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.99f, -0.24f); glScalef(0.52f, 0.12f, 0.10f); glutSolidCube(1.0f); glPopMatrix();

    glPopMatrix();
}


//  Bayangan (karena hidup Galang gelap dipenuhi bayangan, maka ia akan mencari cahaya pada orang yang is cinta

static const float SHADOW_LIGHT[4] = { -15.0f, 12.0f, 10.0f, 0.0f };

static void buildShadowMatrix(float m[16], const float light[4]) {
    float lx=light[0], ly=light[1], lz=light[2], lw=light[3];
    float dot=ly;
    m[ 0]=dot-lx*0; m[ 1]=   -ly*0; m[ 2]=   -lz*0; m[ 3]=   -lw*0;
    m[ 4]=   -lx*1; m[ 5]=dot-ly*1; m[ 6]=   -lz*1; m[ 7]=   -lw*1;
    m[ 8]=   -lx*0; m[ 9]=   -ly*0; m[10]=dot-lz*0; m[11]=   -lw*0;
    m[12]=   -lx*0; m[13]=   -ly*0; m[14]=   -lz*0; m[15]=dot-lw*0;
}
static void beginShadow() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.35f);
}
static void endShadow() {
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}
static void drawPlayerShadow() {
    float m[16]; buildShadowMatrix(m, SHADOW_LIGHT);
    beginShadow();
    glPushMatrix(); glMultMatrixf(m);
    glPushMatrix(); glTranslatef(playerX, playerY+1.2f, playerZ); glRotatef(playerYaw,0,1,0);
    glPushMatrix(); glScalef(0.75f,0.95f,0.38f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0,0.78f,0); glScalef(0.58f,0.58f,0.54f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.18f,-0.5f,0); glRotatef(-swingAngle,1,0,0);
        glPushMatrix(); glTranslatef(0,-0.55f,0); glScalef(0.30f,0.85f,0.30f); glutSolidCube(1.0f); glPopMatrix(); glPopMatrix();
    glPushMatrix(); glTranslatef(0.18f,-0.5f,0); glRotatef(swingAngle,1,0,0);
        glPushMatrix(); glTranslatef(0,-0.55f,0); glScalef(0.30f,0.85f,0.30f); glutSolidCube(1.0f); glPopMatrix(); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.48f,0.30f,0); glRotatef(swingAngle,1,0,0);
        glPushMatrix(); glTranslatef(0,-0.30f,0); glScalef(0.22f,0.60f,0.22f); glutSolidCube(1.0f); glPopMatrix(); glPopMatrix();
    glPushMatrix(); glTranslatef(0.48f,0.30f,0); glRotatef(-swingAngle,1,0,0);
        glPushMatrix(); glTranslatef(0,-0.30f,0); glScalef(0.22f,0.60f,0.22f); glutSolidCube(1.0f); glPopMatrix(); glPopMatrix();
    glPopMatrix(); glPopMatrix();
    endShadow();
}
static void drawTisyaShadow() {
    float m[16]; buildShadowMatrix(m, SHADOW_LIGHT);
    beginShadow();
    glPushMatrix(); glMultMatrixf(m);
    glPushMatrix(); glTranslatef(tisyaX, 1.2f, tisyaZ); glRotatef(tisyaYaw,0,1,0);
    glPushMatrix(); glScalef(0.70f,0.90f,0.36f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0,0.78f,0); glScalef(0.56f,0.56f,0.52f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.14f,-0.55f,0); glScalef(0.28f,0.85f,0.28f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.14f,-0.55f,0); glScalef(0.28f,0.85f,0.28f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix(); glPopMatrix();
    endShadow();
}
static void drawCowShadow() {
    float m[16]; buildShadowMatrix(m, SHADOW_LIGHT);
    beginShadow();
    glPushMatrix(); glMultMatrixf(m);
    glPushMatrix(); glTranslatef(cowX, 0.f, cowZ); glRotatef(cowAngle,0,1,0);
    glTranslatef(0, 1.3f, 0);
    glPushMatrix(); glScalef(0.85f,0.85f,1.4f); glutSolidSphere(1.f,10,7); glPopMatrix();
    glPopMatrix(); glPopMatrix();
    endShadow();
}
static void drawChickenShadow(float x, float z, float angle) {
    float m[16]; buildShadowMatrix(m, SHADOW_LIGHT);
    beginShadow();
    glPushMatrix(); glMultMatrixf(m);
    glPushMatrix(); glTranslatef(x, 0.4f, z); glRotatef(angle,0,1,0); glScalef(0.5f,0.5f,0.5f);
    glPushMatrix(); glScalef(0.8f,0.9f,1.1f); glutSolidSphere(0.5f,8,6); glPopMatrix();
    glPushMatrix(); glTranslatef(0,0.4f,0.4f); glutSolidSphere(0.35f,8,6); glPopMatrix();
    glPopMatrix(); glPopMatrix();
    endShadow();
}


//  Pohon 

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
    // Pohon apel
    Color3 leafBase = hasApples
        ? makeColor(0.20f, 0.55f, 0.12f)
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
            // Apel
            if(hasApples && (i%4==0)){
                setColor(0.85f,0.10f,0.10f);
                glPushMatrix();
                glTranslatef(tx+offX+layerR*cosf(am)*0.80f,
                             layerY+layerH*0.25f,
                             tz+layerR*sinf(am)*0.80f);
                glutSolidSphere(0.13f, 6, 6);
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


// Dinding Bata 

void drawBrickWallX(float fx, float z0, float z1, float y0, float y1,
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


//  Jendela & Pintu

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


//  Rumah

void drawHouse(const House &h){
    float x0=h.cx-h.width*0.5f, x1=h.cx+h.width*0.5f;
    float z0=h.cz-h.depth*0.5f, z1=h.cz+h.depth*0.5f;
    float y0=0.f, y1=h.wallH;
    float ov=0.35f;

    Color3 wF=h.wall, wS=shade(h.wall,0.80f), wB=shade(h.wall,0.65f);

    // 1. Dinding 
    setColor(wF); quad4(x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y1,z1); 
    setColor(wB); quad4(x1,y0,z0, x0,y0,z0, x0,y1,z0, x1,y1,z0); 
    setColor(wS); quad4(x0,y0,z0, x0,y0,z1, x0,y1,z1, x0,y1,z0); 
    setColor(shade(h.wall,0.87f));
              quad4(x1,y0,z1, x1,y0,z0, x1,y1,z0, x1,y1,z1); 

    // 2. Tekstur Bata 
    drawBrickWallZ(z1, x0, x1, y0, y1, wF, +1.f);
    drawBrickWallZ(z0, x0, x1, y0, y1, wB, -1.f);
    drawBrickWallX(x0, z0, z1, y0, y1, wS, -1.f);
    drawBrickWallX(x1, z0, z1, y0, y1, shade(h.wall,0.87f), +1.f);

    // 3. Pintu & Jendela
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

    // 4. Atap 
    float rx0=x0-ov, rx1=x1+ov, rz0=z0-ov, rz1=z1+ov, rY=y1+h.roofH, rX=h.cx;
    setColor(h.roof);          quad4(rx0,y1,rz1, rx0,y1,rz0, rX,rY,rz0, rX,rY,rz1);
    setColor(shade(h.roof,0.68f)); quad4(rx1,y1,rz0, rx1,y1,rz1, rX,rY,rz1, rX,rY,rz0);
    setColor(shade(h.wall,0.92f)); tri3(rx0,y1,rz1, rx1,y1,rz1, rX,rY,rz1);
    setColor(shade(h.wall,0.72f)); tri3(rx1,y1,rz0, rx0,y1,rz0, rX,rY,rz0);
}


//  Jalan, Tanah, Langit

void drawRoad(){
    float rw=6.0f,zN=20.0f,zF=-150.0f,sw=1.4f;
    float ts=0.1f; 
    enableTex(texRoad); glNormal3f(0,1,0);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);       glVertex3f(-rw*.5f,0.005f,zN);
        glTexCoord2f(1,0);       glVertex3f( rw*.5f,0.005f,zN);
        glTexCoord2f(1,(zN-zF)*ts); glVertex3f( rw*.5f,0.005f,zF);
        glTexCoord2f(0,(zN-zF)*ts); glVertex3f(-rw*.5f,0.005f,zF);
    glEnd(); disableTex();
    enableTex(texDirt); glNormal3f(0,1,0);
    for(int s=-1;s<=1;s+=2){
        float sx0=s*rw*.5f,sx1=s*(rw*.5f+sw);
        glBegin(GL_QUADS);
            glTexCoord2f(0,0);         glVertex3f(sx0,0.003f,zN);
            glTexCoord2f(1,0);         glVertex3f(sx1,0.003f,zN);
            glTexCoord2f(1,(zN-zF)*ts);glVertex3f(sx1,0.003f,zF);
            glTexCoord2f(0,(zN-zF)*ts);glVertex3f(sx0,0.003f,zF);
        glEnd();
    }
    disableTex();
}

void drawGround(){
    enableTex(texDirt); glNormal3f(0,1,0);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);   glVertex3f(-300,0,25);
        glTexCoord2f(60,0);  glVertex3f( 300,0,25);
        glTexCoord2f(60,45); glVertex3f( 300,0,-200);
        glTexCoord2f(0,45);  glVertex3f(-300,0,-200);
    glEnd(); disableTex();
    enableTex(texGrass); glNormal3f(0,1,0);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);   glVertex3f(-100,0.002f,22);
        glTexCoord2f(20,0);  glVertex3f( 100,0.002f,22);
        glTexCoord2f(20,40); glVertex3f( 100,0.002f,-180);
        glTexCoord2f(0,40);  glVertex3f(-100,0.002f,-180);
    glEnd(); disableTex();
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


//  Sapi

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

        float moveRad = cowAngle * D2R;
        cowX += sinf(moveRad) * cowMoveSpeed;
        cowZ += cosf(moveRad) * cowMoveSpeed;

        walkPhase += 0.08f;
    }
}


//  HUD

static void drawText2D(float x, float y, const char* text, float r, float g, float b) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluOrtho2D(0, viewport[2], 0, viewport[3]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(x+2, y-2);
    for(const char* c = text; *c != '\0'; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for(const char* c = text; *c != '\0'; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void drawHUD() {
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    float W = (float)viewport[2];
    float H = (float)viewport[3];

    if(milkPrompt && !isMillking && currentMission == MISSION_SUSU && missionActive) {
        const char* prompt = "[E] Perah Sapi";
        drawText2D(W * 0.5f - 70.0f, H * 0.22f, prompt, 1.0f, 0.95f, 0.3f);
    }
    if(nearGateSapi && currentMission == MISSION_SUSU) {
        const char* prompt = gateSapiOpen ? "[Q] Tutup Gerbang Sapi" : "[Q] Buka Gerbang Sapi";
        drawText2D(W * 0.5f - 100.0f, H * 0.28f, prompt, 0.3f, 1.0f, 0.5f);
    }
    if(nearGateAyam && (currentMission == MISSION_TELUR)) {
        const char* prompt = gateAyamOpen ? "[Q] Tutup Gerbang Ayam" : "[Q] Buka Gerbang Ayam";
        drawText2D(W * 0.5f - 100.0f, H * 0.28f, prompt, 0.3f, 1.0f, 0.5f);
    }
    if(isMillking && currentMission == MISSION_SUSU) {
        drawText2D(W * 0.5f - 85.0f, H * 0.22f, "Sedang memerah...", 0.6f, 1.0f, 0.6f);
    }

    // Nyawa 
    char liveBuf[32];
    sprintf(liveBuf, "Nyawa: %d", playerLives);
    drawText2D(20.0f, H - 36.0f, liveBuf, 1.0f, 0.3f, 0.3f);

    // Level Hubungan
    const char* relText = "Status: Stranger";
    if(relationLevel == REL_PACARAN)      relText = "Status: Pacaran";
    else if(relationLevel == REL_PERTUNANGAN) relText = "Status: Pertunangan";
    else if(relationLevel == REL_PERNIKAHAN)  relText = "Status: Menikah";
    drawText2D(20.0f, H - 60.0f, relText, 1.0f, 0.75f, 0.85f);

    // Mission HUD
    if(missionActive && gameState == GS_PLAYING) {
        if(currentMission == MISSION_APEL) {
            char abuf[64];
            int sisa = (int)missionTimer;
            sprintf(abuf, "Apel: %d/%d  |  Waktu: %ds", applesCollected, TOTAL_APPLES, sisa);
            drawText2D(W * 0.5f - 120.0f, H - 36.0f, abuf, 1.0f, 1.0f, 0.3f);
            drawText2D(W*0.5f-100.0f, H*0.22f, "[E] Ambil Apel (dekatkan ke apel)", 1.0f, 0.95f, 0.3f);
        } else if(currentMission == MISSION_TELUR) {
            char ebuf[64];
            int sisa = (int)missionTimer;
            float timerColor = (sisa <= 20) ? 1.0f : 1.0f;
            float timerG     = (sisa <= 20) ? 0.2f : 1.0f;
            sprintf(ebuf, "Telur: %d/%d  |  Waktu: %ds", eggsCollected, TOTAL_EGGS, sisa);
            drawText2D(W * 0.5f - 120.0f, H - 36.0f, ebuf, timerColor, timerG, 0.3f);
            drawText2D(W*0.5f-120.0f, H*0.22f, "[E] Ambil Telur (dekatkan ke telur)", 1.0f, 0.95f, 0.3f);
        } else if(currentMission == MISSION_SUSU) {
            char sbuf[64];
            int sisa = (int)missionTimer;
            float timerColor = (sisa <= 30) ? 1.0f : 1.0f;
            float timerG     = (sisa <= 30) ? 0.2f : 1.0f;
            sprintf(sbuf, "Susu: %d/%d ember  |  Waktu: %ds", milkCount, TARGET_MILK, sisa);
            drawText2D(W * 0.5f - 120.0f, H - 36.0f, sbuf, timerColor, timerG, 0.3f);
        }
    }
    
    if(showLevelUpBanner) {
	    int vp[4]; glGetIntegerv(GL_VIEWPORT, vp);
	    float W = (float)vp[2], H = (float)vp[3];
	    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
	    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
	    gluOrtho2D(0,W,0,H);
	    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
	    glColor4f(0,0,0,0.85f);
	    glBegin(GL_QUADS);
	        glVertex2f(0,0); glVertex2f(W,0);
	        glVertex2f(W,H); glVertex2f(0,H);
	    glEnd();
	    const char* msg = "Status kamu naik!";
	    if(relationLevel == REL_PACARAN)      msg = "Selamat! Kalian kini berpacaran!";
	    else if(relationLevel == REL_PERTUNANGAN) msg = "Selamat! Kalian kini bertunangan!";
	    else if(relationLevel == REL_PERNIKAHAN)  msg = "Selamat! Kalian kini menikah!";
	    drawText2D(W*0.5f-150.0f, H*0.5f, msg, 1.0f, 0.85f, 0.3f);
	    glMatrixMode(GL_PROJECTION); glPopMatrix();
	    glMatrixMode(GL_MODELVIEW); glPopMatrix();
	    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
	}
	
	if(showRumahBanner) {
	    int vp[4]; glGetIntegerv(GL_VIEWPORT, vp);
	    float W = (float)vp[2], H = (float)vp[3];
	    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
	    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
	    gluOrtho2D(0,W,0,H);
	    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
	    glColor4f(0,0,0,0.92f);
	    glBegin(GL_QUADS);
	        glVertex2f(0,0); glVertex2f(W,0);
	        glVertex2f(W,H); glVertex2f(0,H);
	    glEnd();
	    drawText2D(W*0.5f-70.0f, H*0.5f, "Rumah Bro", 1.0f, 0.85f, 0.0f);
	    glMatrixMode(GL_PROJECTION); glPopMatrix();
	    glMatrixMode(GL_MODELVIEW); glPopMatrix();
	    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
	}

    // bicara dengan Tisya
    if(nearTisya && !dialogActive && !showLevelUpBanner &&  (gameState == GS_PLAYING || gameState == GS_MISSION_SUCCESS)) {
        drawText2D(W*0.5f - 130.0f, H*0.18f, "[E] Bicara dengan Tisya", 0.9f, 0.8f, 1.0f);
    }
    
    if(nearRumahBro && !showRumahBanner)
	    drawText2D(W*0.5f - 100.0f, H*0.13f, "[Q] Masuk Rumah Bro", 0.9f, 0.8f, 1.0f);
	if(insideRumahBro) {
	    float dx = playerX - RUMAH_INTERIOR_X;
	    float dz = playerZ - RUMAH_DOOR_Z_OUT;
	    if(sqrtf(dx*dx + dz*dz) < RUMAH_DOOR_RANGE)
	        drawText2D(W*0.5f - 60.0f, H*0.13f, "[E] Keluar", 0.9f, 0.8f, 1.0f);
}

    // main setelah story
    if(gameState == GS_STORY && !dialogActive) {
        drawText2D(W*0.5f - 110.0f, H*0.14f, "[E] Mulai Misi", 0.4f, 1.0f, 0.5f);
    }

    // Kontrol
    drawText2D(20.0f, 30.0f, "WASD/Arrow=Gerak  Space=Lompat  R=Reset  E=Aksi  Q=Gerbang", 0.75f, 0.75f, 0.75f);

    // Dialog Box
    if(dialogActive && dialogIndex < dialogCount) {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix(); glLoadIdentity();
        gluOrtho2D(0, W, 0, H);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix(); glLoadIdentity();

        float bx0 = W*0.08f, bx1 = W*0.92f;
        float by0 = H*0.05f, by1 = H*0.22f;
        // shadow
        glColor4f(0,0,0,0.7f);
        glBegin(GL_QUADS);
            glVertex2f(bx0+3, by0-3); glVertex2f(bx1+3, by0-3);
            glVertex2f(bx1+3, by1-3); glVertex2f(bx0+3, by1-3);
        glEnd();
        // box
        glColor3f(0.10f, 0.07f, 0.15f);
        glBegin(GL_QUADS);
            glVertex2f(bx0,by0); glVertex2f(bx1,by0);
            glVertex2f(bx1,by1); glVertex2f(bx0,by1);
        glEnd();
        // border
        glColor3f(0.85f, 0.65f, 0.90f);
        glLineWidth(2.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(bx0,by0); glVertex2f(bx1,by0);
            glVertex2f(bx1,by1); glVertex2f(bx0,by1);
        glEnd();
        glLineWidth(1.0f);

        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW);  glPopMatrix();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);

        // Teks speaker + isi
        const DialogLine &dl = dialogLines[dialogIndex];
        char speakerBuf[64];
        sprintf(speakerBuf, "%s:", dl.speaker);

        float tx = bx0 + 18.0f;
        float ty = by1 - 28.0f;

        // Warna speaker
        float sr=1,sg=1,sb=0.3f;
        if(strcmp(dl.speaker,"Tisya")==0)  { sr=1.0f; sg=0.7f; sb=0.9f; }
        if(strcmp(dl.speaker,"Gilang")==0) { sr=0.5f; sg=0.8f; sb=1.0f; }
        if(strcmp(dl.speaker,"Narator")==0){ sr=0.9f; sg=0.9f; sb=0.6f; }

        drawText2D(tx, ty, speakerBuf, sr, sg, sb);
        drawText2D(tx, ty - 28.0f, dl.text, 1.0f, 1.0f, 1.0f);

        // Progress & petunjuk
        char progBuf[32];
        sprintf(progBuf, "(%d/%d)", dialogIndex+1, dialogCount);
        drawText2D(bx1 - 80.0f, by0 + 8.0f, progBuf, 0.6f, 0.6f, 0.6f);
        drawText2D(bx1 - 180.0f, by0 + 8.0f, "[E] Lanjut", 0.8f, 1.0f, 0.8f);
    }

    // Level Up Banner
    if(gameState == GS_LEVEL_UP) {
        heartAnim += 0.05f;
        float scale = 1.0f + 0.15f * sinf(heartAnim);
        const char* lvlText = "";
        if(relationLevel == REL_PACARAN)          lvlText = "LEVEL UP -- PACARAN <3";
        else if(relationLevel == REL_PERTUNANGAN) lvlText = "LEVEL UP -- PERTUNANGAN <3";
        else if(relationLevel == REL_PERNIKAHAN)  lvlText = "LEVEL UP -- PERNIKAHAN <3";
        else                                      lvlText = "SELAMAT!";
        (void)scale;
        drawText2D(W*0.5f - 150.0f, H*0.55f, lvlText, 1.0f, 0.4f, 0.7f);
        drawText2D(W*0.5f - 120.0f, H*0.45f, "[E] Lanjutkan", 0.9f, 1.0f, 0.9f);
        if(relationLevel == REL_PERNIKAHAN)
            drawText2D(W*0.5f - 160.0f, H*0.35f, "Selamat! Gilang & Tisya menikah!", 1.0f, 0.85f, 0.5f);
    }

    // Game Over
    if(gameState == GS_GAME_OVER) {
        drawText2D(W*0.5f - 80.0f, H*0.60f, "GAME OVER", 1.0f, 0.2f, 0.2f);
        drawText2D(W*0.5f - 200.0f, H*0.48f,
            "Gilang kehilangan kesempatan mendapatkan hati Tisya.", 1.0f, 0.8f, 0.8f);
        drawText2D(W*0.5f - 140.0f, H*0.38f,
            "Kadang cinta datang hanya sekali...", 1.0f, 0.8f, 0.8f);
        drawText2D(W*0.5f - 80.0f, H*0.28f, "[R] Main Lagi", 0.8f, 1.0f, 0.8f);
    }

    // Win
    if(gameState == GS_WIN) {
        drawText2D(W*0.5f - 180.0f, H*0.60f, "SELAMAT! GILANG & TISYA MENIKAH!", 1.0f, 0.85f, 0.4f);
        drawText2D(W*0.5f - 80.0f, H*0.45f, "[R] Main Lagi", 0.9f, 1.0f, 0.9f);
    }
}


//  Ayam

static unsigned int chickenRandState = 12345;
static unsigned int chickenRand() {
    chickenRandState ^= chickenRandState << 13;
    chickenRandState ^= chickenRandState >> 17;
    chickenRandState ^= chickenRandState << 5;
    return chickenRandState;
}

void updateChickens() {
    for(int i = 0; i < NUM_CHICKENS; i++) {
        Chicken &ch = chickens[i];
        ch.turnTimer--;
        if(ch.turnTimer <= 0) {
            float randAngle = (float)(chickenRand() % 360);
            ch.turnTarget = randAngle;
            ch.turnTimer = 20 + (int)(chickenRand() % 40);
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


//  Scene Utama

void drawScene(){
    drawSky();
    drawGround();
    drawRoad();

    for(int i=0;i<(int)houses.size();i++)
        drawHouse(houses[i]);

    {
        float kx =  22.0f;
        float kz = -35.0f;
        TONG_X = kx + 2.5f;
        TONG_Z = kz + 1.0f;
        drawKandangSapi(kx, kz);
        drawTong();
        drawCow();
    }

    {
        float ax_area = CHICKEN_CX;
        float az_area = CHICKEN_CZ;
        drawKandangAyam(ax_area, az_area);
        for(int i = 0; i < NUM_CHICKENS; i++) {
            drawSingleChicken(chickens[i].x, chickens[i].z,
                              chickens[i].angle, chickens[i].legAnim);
        }
    }

    srand(55);
    for(int row=0;row<4;row++){
        for(int col=0;col<5;col++){
            float ax = -15.0f - col*5.0f + rnd(-0.5f,0.5f);
            float az = -20.0f - row*6.0f + rnd(-0.5f,0.5f);
            int idx=row*5+col;
            if((int)treesApple.size()<=idx){ TreePos tp; tp.x=ax; tp.z=az; treesApple.push_back(tp); }
            drawTreeNatural(ax, az, (unsigned)(row*10+col), true);
        }
    }

    for(int i=0;i<(int)houses.size();i+=2){
        const House &h=houses[i];
        drawFence(h.cx-h.width*0.5f, h.cz+h.depth*0.5f+0.08f,
                  h.cx+h.width*0.5f, h.cz+h.depth*0.5f+0.08f, 8);
    }

    // Apel jatuh (Level 1)
    if(currentMission == MISSION_APEL && applesInitialized) {
        for(int i = 0; i < TOTAL_APPLES; i++) {
            if(!appleItems[i].collected)
                drawGroundApple(appleItems[i].x, appleItems[i].y, appleItems[i].z);
        }
    }

    // Telur (Level 2)
    if(currentMission == MISSION_TELUR && eggsInitialized) {
        for(int i = 0; i < TOTAL_EGGS; i++) {
            if(!eggItems[i].collected)
                drawEgg(eggItems[i].x, eggItems[i].z);
        }
    }
    
	drawCowShadow();
	for(int i=0;i<NUM_CHICKENS;i++)
	    drawChickenShadow(chickens[i].x, chickens[i].z, chickens[i].angle);
	drawPlayerShadow();
	drawTisyaShadow();

    drawPlayer();
    drawTisya();
}


//  Setup Pencahayaan & Fog

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

void setupInteriorLighting() {
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_FOG);

    // Lampu di tengah ruangan
    glEnable(GL_LIGHT2);
    float cx = RUMAH_INTERIOR_X, cz = RUMAH_INTERIOR_Z;
    GLfloat lampPos[] = { cx, 5.0f, cz, 1.0f }; 
    GLfloat lampAmb[] = { 0.15f, 0.10f, 0.05f, 1.0f };
    GLfloat lampDif[] = { 1.00f, 0.85f, 0.60f, 1.0f }; 
    GLfloat lampSpc[] = { 0.20f, 0.15f, 0.10f, 1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION,  lampPos);
    glLightfv(GL_LIGHT2, GL_AMBIENT,   lampAmb);
    glLightfv(GL_LIGHT2, GL_DIFFUSE,   lampDif);
    glLightfv(GL_LIGHT2, GL_SPECULAR,  lampSpc);
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION,  0.5f);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION,    0.05f);
    glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.01f);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void drawRumahInterior() {
    float cx = RUMAH_INTERIOR_X, cz = RUMAH_INTERIOR_Z;
    float RW = 20.0f, RH = 6.0f, RD = 20.0f;

    // bata
    Color3 brickFace  = makeColor(0.72f, 0.36f, 0.18f);
    Color3 brickDark  = makeColor(0.55f, 0.25f, 0.10f);
    Color3 mortar     = makeColor(0.82f, 0.78f, 0.72f);
    Color3 flrA       = makeColor(0.40f, 0.26f, 0.12f);
    Color3 flrB       = makeColor(0.52f, 0.34f, 0.18f);
    Color3 doorC      = makeColor(0.32f, 0.18f, 0.08f);
    Color3 ceilColor  = makeColor(0.45f, 0.22f, 0.10f);

    // Lantai papan
    int strips = 20;
    for(int i = 0; i < strips; i++) {
        float z0s = cz - RD/2 + i * (RD / strips);
        float z1s = z0s + RD / strips;
        setColor(i % 2 == 0 ? flrA : flrB);
        glNormal3f(0,1,0);
        glBegin(GL_QUADS);
            glVertex3f(cx-RW/2, 0.005f, z0s);
            glVertex3f(cx+RW/2, 0.005f, z0s);
            glVertex3f(cx+RW/2, 0.005f, z1s);
            glVertex3f(cx-RW/2, 0.005f, z1s);
        glEnd();
    }
    
    // Dinding kiri 
    {
        float wx = cx - RW/2;
        float bH = 0.20f, bL = 0.42f, gap = 0.03f;
        int rows = (int)(RH / (bH+gap)) + 1;
        int cols = (int)(RD / (bL+gap)) + 2;
        setColor(mortar);
        glNormal3f(1,0,0);
        glBegin(GL_QUADS);
            glVertex3f(wx, 0, cz-RD/2); glVertex3f(wx, 0, cz+RD/2);
            glVertex3f(wx, RH, cz+RD/2); glVertex3f(wx, RH, cz-RD/2);
        glEnd();
        for(int r=0;r<rows;r++){
            float y0=r*(bH+gap), y1=y0+bH; if(y0>=RH) break; if(y1>RH) y1=RH;
            float off = (r%2==0)?0:(bL+gap)*0.5f;
            Color3 bc = (r%3==0)?brickDark:brickFace;
            for(int c=-1;c<cols;c++){
                float z0b=cz-RD/2+c*(bL+gap)-off+gap*0.5f, z1b=z0b+bL;
                if(z1b<=cz-RD/2||z0b>=cz+RD/2) continue;
                if(z0b<cz-RD/2) z0b=cz-RD/2; if(z1b>cz+RD/2) z1b=cz+RD/2;
                float var=0.88f+0.18f*((float)((r*11+c*7)%7)/6.f);
                setColor(shade(bc,var));
                glNormal3f(1,0,0);
                glBegin(GL_QUADS);
                    glVertex3f(wx+0.02f,y0,z0b); glVertex3f(wx+0.02f,y0,z1b);
                    glVertex3f(wx+0.02f,y1,z1b); glVertex3f(wx+0.02f,y1,z0b);
                glEnd();
            }
        }
    }

    // Dinding kanan
    {
        float wx = cx + RW/2;
        float bH = 0.20f, bL = 0.42f, gap = 0.03f;
        int rows = (int)(RH / (bH+gap)) + 1;
        int cols = (int)(RD / (bL+gap)) + 2;
        setColor(mortar);
        glNormal3f(-1,0,0);
        glBegin(GL_QUADS);
            glVertex3f(wx, 0, cz-RD/2); glVertex3f(wx, 0, cz+RD/2);
            glVertex3f(wx, RH, cz+RD/2); glVertex3f(wx, RH, cz-RD/2);
        glEnd();
        for(int r=0;r<rows;r++){
            float y0=r*(bH+gap), y1=y0+bH; if(y0>=RH) break; if(y1>RH) y1=RH;
            float off = (r%2==0)?0:(bL+gap)*0.5f;
            Color3 bc = (r%3==0)?brickDark:brickFace;
            for(int c=-1;c<cols;c++){
                float z0b=cz-RD/2+c*(bL+gap)-off+gap*0.5f, z1b=z0b+bL;
                if(z1b<=cz-RD/2||z0b>=cz+RD/2) continue;
                if(z0b<cz-RD/2) z0b=cz-RD/2; if(z1b>cz+RD/2) z1b=cz+RD/2;
                float var=0.88f+0.18f*((float)((r*11+c*7)%7)/6.f);
                setColor(shade(bc,var));
                glNormal3f(-1,0,0);
                glBegin(GL_QUADS);
                    glVertex3f(wx-0.02f,y0,z0b); glVertex3f(wx-0.02f,y0,z1b);
                    glVertex3f(wx-0.02f,y1,z1b); glVertex3f(wx-0.02f,y1,z0b);
                glEnd();
            }
        }
    }

    // Dinding belakang
    {
        float fz = cz - RD/2;
        float bH = 0.20f, bL = 0.42f, gap = 0.03f;
        int rows = (int)(RH / (bH+gap)) + 1;
        int cols = (int)(RW / (bL+gap)) + 2;
        setColor(mortar);
        glNormal3f(0,0,1);
        glBegin(GL_QUADS);
            glVertex3f(cx-RW/2,0,fz); glVertex3f(cx+RW/2,0,fz);
            glVertex3f(cx+RW/2,RH,fz); glVertex3f(cx-RW/2,RH,fz);
        glEnd();
        for(int r=0;r<rows;r++){
            float y0=r*(bH+gap), y1=y0+bH; if(y0>=RH) break; if(y1>RH) y1=RH;
            float off = (r%2==0)?0:(bL+gap)*0.5f;
            Color3 bc = (r%3==0)?brickDark:brickFace;
            for(int c=-1;c<cols;c++){
                float x0b=cx-RW/2+c*(bL+gap)-off+gap*0.5f, x1b=x0b+bL;
                if(x1b<=cx-RW/2||x0b>=cx+RW/2) continue;
                if(x0b<cx-RW/2) x0b=cx-RW/2; if(x1b>cx+RW/2) x1b=cx+RW/2;
                float var=0.88f+0.18f*((float)((r*11+c*7)%7)/6.f);
                setColor(shade(bc,var));
                glNormal3f(0,0,1);
                glBegin(GL_QUADS);
                    glVertex3f(x0b,y0,fz+0.05f); glVertex3f(x1b,y0,fz+0.05f);
                    glVertex3f(x1b,y1,fz+0.05f); glVertex3f(x0b,y1,fz+0.05f);
                glEnd();
            }
        }
    }

    // Dinding depan kiri pintu
    {
        float fz = cz + RD/2;
        float x0seg = cx-RW/2, x1seg = cx-1.0f;
        float bH = 0.20f, bL = 0.42f, gap = 0.03f;
        int rows = (int)(RH / (bH+gap)) + 1;
        int cols = (int)((x1seg-x0seg) / (bL+gap)) + 2;
        setColor(mortar); glNormal3f(0,0,-1);
        glBegin(GL_QUADS);
            glVertex3f(x0seg,0,fz); glVertex3f(x1seg,0,fz);
            glVertex3f(x1seg,RH,fz); glVertex3f(x0seg,RH,fz);
        glEnd();
        for(int r=0;r<rows;r++){
            float y0=r*(bH+gap),y1=y0+bH; if(y0>=RH)break; if(y1>RH)y1=RH;
            float off=(r%2==0)?0:(bL+gap)*0.5f;
            Color3 bc=(r%3==0)?brickDark:brickFace;
            for(int c=-1;c<cols;c++){
                float x0b=x0seg+c*(bL+gap)-off+gap*0.5f,x1b=x0b+bL;
                if(x1b<=x0seg||x0b>=x1seg)continue;
                if(x0b<x0seg)x0b=x0seg; if(x1b>x1seg)x1b=x1seg;
                float var=0.88f+0.18f*((float)((r*11+c*7)%7)/6.f);
                setColor(shade(bc,var)); glNormal3f(0,0,-1);
                glBegin(GL_QUADS);
                    glVertex3f(x0b,y0,fz-0.05f); glVertex3f(x1b,y0,fz-0.05f);
                    glVertex3f(x1b,y1,fz-0.05f); glVertex3f(x0b,y1,fz-0.05f);
                glEnd();
            }
        }
    }
    // Dinding depan kanan pintu
    {
        float fz = cz + RD/2;
        float x0seg = cx+1.0f, x1seg = cx+RW/2;
        float bH = 0.20f, bL = 0.42f, gap = 0.03f;
        int rows = (int)(RH / (bH+gap)) + 1;
        int cols = (int)((x1seg-x0seg) / (bL+gap)) + 2;
        setColor(mortar); glNormal3f(0,0,-1);
        glBegin(GL_QUADS);
            glVertex3f(x0seg,0,fz); glVertex3f(x1seg,0,fz);
            glVertex3f(x1seg,RH,fz); glVertex3f(x0seg,RH,fz);
        glEnd();
        for(int r=0;r<rows;r++){
            float y0=r*(bH+gap),y1=y0+bH; if(y0>=RH)break; if(y1>RH)y1=RH;
            float off=(r%2==0)?0:(bL+gap)*0.5f;
            Color3 bc=(r%3==0)?brickDark:brickFace;
            for(int c=-1;c<cols;c++){
                float x0b=x0seg+c*(bL+gap)-off+gap*0.5f,x1b=x0b+bL;
                if(x1b<=x0seg||x0b>=x1seg)continue;
                if(x0b<x0seg)x0b=x0seg; if(x1b>x1seg)x1b=x1seg;
                float var=0.88f+0.18f*((float)((r*11+c*7)%7)/6.f);
                setColor(shade(bc,var)); glNormal3f(0,0,-1);
                glBegin(GL_QUADS);
                    glVertex3f(x0b,y0,fz-0.05f); glVertex3f(x1b,y0,fz-0.05f);
                    glVertex3f(x1b,y1,fz-0.05f); glVertex3f(x0b,y1,fz-0.05f);
                glEnd();
            }
        }
    }
    // Dinding depan atas pintu
    {
        float fz = cz + RD/2;
        float x0seg = cx-1.0f, x1seg = cx+1.0f;
        float y0seg = 2.5f, y1seg = RH;
        float bH = 0.20f, bL = 0.42f, gap = 0.03f;
        int rows = (int)((y1seg-y0seg) / (bH+gap)) + 1;
        int cols = (int)((x1seg-x0seg) / (bL+gap)) + 2;
        setColor(mortar); glNormal3f(0,0,-1);
        glBegin(GL_QUADS);
            glVertex3f(x0seg,y0seg,fz); glVertex3f(x1seg,y0seg,fz);
            glVertex3f(x1seg,y1seg,fz); glVertex3f(x0seg,y1seg,fz);
        glEnd();
        for(int r=0;r<rows;r++){
            float y0=y0seg+r*(bH+gap),y1=y0+bH; if(y0>=y1seg)break; if(y1>y1seg)y1=y1seg;
            float off=(r%2==0)?0:(bL+gap)*0.5f;
            Color3 bc=(r%3==0)?brickDark:brickFace;
            for(int c=-1;c<cols;c++){
                float x0b=x0seg+c*(bL+gap)-off+gap*0.5f,x1b=x0b+bL;
                if(x1b<=x0seg||x0b>=x1seg)continue;
                if(x0b<x0seg)x0b=x0seg; if(x1b>x1seg)x1b=x1seg;
                float var=0.88f+0.18f*((float)((r*11+c*7)%7)/6.f);
                setColor(shade(bc,var)); glNormal3f(0,0,-1);
                glBegin(GL_QUADS);
                    glVertex3f(x0b,y0,fz-0.05f); glVertex3f(x1b,y0,fz-0.05f);
                    glVertex3f(x1b,y1,fz-0.05f); glVertex3f(x0b,y1,fz-0.05f);
                glEnd();
            }
        }
    }

    // Pintu
    setColor(doorC);
    glNormal3f(0,0,-1);
    glBegin(GL_QUADS);
        glVertex3f(cx-1.0f,0,cz+RD/2-0.02f); glVertex3f(cx+1.0f,0,cz+RD/2-0.02f);
        glVertex3f(cx+1.0f,2.5f,cz+RD/2-0.02f); glVertex3f(cx-1.0f,2.5f,cz+RD/2-0.02f);
    glEnd();

    // Langit-langit
    setColor(ceilColor);
    glNormal3f(0,-1,0);
    glBegin(GL_QUADS);
        glVertex3f(cx-RW/2,RH,cz-RD/2); glVertex3f(cx+RW/2,RH,cz-RD/2);
        glVertex3f(cx+RW/2,RH,cz+RD/2); glVertex3f(cx-RW/2,RH,cz+RD/2);
    glEnd();

    // Ground luar
    setColor(0.4f, 0.6f, 0.3f);
    glNormal3f(0,1,0);
    glBegin(GL_QUADS);
        glVertex3f(cx-50,-0.01f,cz-50); glVertex3f(cx+50,-0.01f,cz-50);
        glVertex3f(cx+50,-0.01f,cz+50); glVertex3f(cx-50,-0.01f,cz+50);
    glEnd();
    
    // lampu 
    float cx2 = cx, cz2 = cz;
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.98f, 0.80f);
    glPushMatrix();
        glTranslatef(cx2, 5.6f, cz2);
        glutSolidSphere(0.18f, 10, 8);
    glPopMatrix();
    glEnable(GL_LIGHTING);

    // Bayangan player dari lampu interior
    float lampX = cx, lampY = 5.0f, lampZ = cz;
    float shadowLight[4] = { lampX, lampY, lampZ, 1.0f };

    float lx=lampX, ly=lampY, lz=lampZ, lw=1.0f;
    float dot = 0*lx + 1*ly + 0*lz + 0*lw; 
    float sm[16];
    sm[ 0]=dot-lx*0; sm[ 1]=   -ly*0; sm[ 2]=   -lz*0; sm[ 3]=   -lw*0;
    sm[ 4]=   -lx*1; sm[ 5]=dot-ly*1; sm[ 6]=   -lz*1; sm[ 7]=   -lw*1;
    sm[ 8]=   -lx*0; sm[ 9]=   -ly*0; sm[10]=dot-lz*0; sm[11]=   -lw*0;
    sm[12]=   -lx*0; sm[13]=   -ly*0; sm[14]=   -lz*0; sm[15]=dot-lw*0;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.40f);

    glPushMatrix();
    glMultMatrixf(sm);
    glPushMatrix();
    glTranslatef(playerX, playerY+1.2f, playerZ);
    glRotatef(playerYaw, 0,1,0);
    glPushMatrix(); glScalef(0.75f,0.95f,0.38f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0,0.78f,0); glScalef(0.58f,0.58f,0.54f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.18f,-0.5f,0); glRotatef(-swingAngle,1,0,0);
        glPushMatrix(); glTranslatef(0,-0.55f,0); glScalef(0.30f,0.85f,0.30f); glutSolidCube(1.0f); glPopMatrix(); glPopMatrix();
    glPushMatrix(); glTranslatef(0.18f,-0.5f,0); glRotatef(swingAngle,1,0,0);
        glPushMatrix(); glTranslatef(0,-0.55f,0); glScalef(0.30f,0.85f,0.30f); glutSolidCube(1.0f); glPopMatrix(); glPopMatrix();
    glPopMatrix();
    glPopMatrix();

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}


//  GLUT Callbacks

void display(){
    glClearColor(0.72f,0.62f,0.55f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    cam.apply();
    setupEnvironment();
    if(insideRumahBro) { setupInteriorLighting(); drawRumahInterior(); drawPlayer(); }
	else drawScene();
    drawHUD();
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
        // Reset
        playerX=0.0f; playerY=groundY; playerZ=12.0f;
        playerYaw=180.0f;
        velocityY=0.0f; isGrounded=true;
        cam.angleAroundPlayer=0.0f; cam.pitch=20.0f; cam.distance=6.0f;
        tisyaX = 0.0f; tisyaZ = 9.0f; tisyaYaw = 180.0f;

        if(gameState == GS_GAME_OVER || gameState == GS_WIN) {
            // Full reset
            playerLives    = 3;
            currentMission = MISSION_APEL;
            relationLevel  = REL_STRANGER;
            applesCollected = 0; applesInitialized = false;
            eggsCollected   = 0; eggsInitialized   = false;
            milkCount       = 0;
            missionActive   = false;
            missionTimer    = 60.0f;
            cowIsMoving     = true;
            gateSapiOpen    = false; gateSapiAngle = 0;
            gateAyamOpen    = false; gateAyamAngle = 0;
            clearDialog();
            showStoryBefore();
        }
        return;
    }

    if(key=='q'||key=='Q') {
        if(nearGateSapi) gateSapiOpen = !gateSapiOpen;
        if(nearGateAyam) gateAyamOpen = !gateAyamOpen;
		if(nearRumahBro && !showRumahBanner) {
		    showRumahBanner  = true;
		    rumahBannerTimer = 1.5f;
		}
    }

    if(key=='e'||key=='E') {
        // Dialog
        if(dialogActive) {
            dialogIndex++;
            if(dialogIndex >= dialogCount) {
                // Dialog selesai
                clearDialog();

                if(gameState == GS_STORY) {
                    // Mulai gameplay
                    missionActive = true;
                    gameState = GS_PLAYING;
                    missionTimer = 60.0f;
                    timerTicks   = 0;
                    if(currentMission == MISSION_APEL && !applesInitialized) initAppleItems();
                    if(currentMission == MISSION_TELUR && !eggsInitialized)  initEggItems();
                } else if(gameState == GS_MISSION_SUCCESS) {
    				if(currentMission == MISSION_APEL) {
        				relationLevel  = REL_PACARAN;
        				currentMission = MISSION_TELUR;
    				} else if(currentMission == MISSION_TELUR) {
        				relationLevel  = REL_PERTUNANGAN;
        				currentMission = MISSION_SUSU;
        				milkCount      = 0;
    				} else if(currentMission == MISSION_SUSU) {
        				relationLevel = REL_PERNIKAHAN;
        				gameState = GS_WIN;
        				missionActive = false;
        				return;
    				}
    				showLevelUpBanner = true;
					levelUpBannerTimer = 1.5f;
    				gameState     = GS_PLAYING;
    				missionActive = false;
    				clearDialog();
                } else if(gameState == GS_MISSION_FAIL) {
                    playerLives--;
                    if(playerLives <= 0) {
                        gameState = GS_GAME_OVER;
                        missionActive = false;
                    } else {
                        // Coba lagi misi yang sama
                        applesCollected = 0; if(applesInitialized){ initAppleItems(); }
                        eggsCollected   = 0; if(eggsInitialized)  { initEggItems();   }
                        milkCount       = 0;
                        missionActive   = false;
                        showRetryDialog();
                    }
                }
            }
            return;
        }

        // Level Up: lanjut ke story berikut
        if(gameState == GS_LEVEL_UP) {
            showStoryBefore();
            return;
        }

        // Gameplay: aksi kontekstual
        if(gameState == GS_PLAYING|| gameState == GS_MISSION_SUCCESS) {

            // Bicara Tisya
            if(nearTisya && !showLevelUpBanner) {
			    if(!dialogActive) {
			        if(gameState == GS_MISSION_SUCCESS) {
			            showSuccessDialog();
			        } else if(!missionActive) {
					    if(missionTimer <= 0.0f) {
					        showFailDialog();
					    } else {
					        showStoryBefore();
					    }
			        }
			    }
			    return;
			}
            
			if(!missionActive) return;

            // Level 1: ambil apel
            if(currentMission == MISSION_APEL) {
                for(int i = 0; i < TOTAL_APPLES; i++) {
                    if(appleItems[i].collected) continue;
                    float dx = playerX - appleItems[i].x;
                    float dz = playerZ - appleItems[i].z;
                    float dist = sqrtf(dx*dx+dz*dz);
                    if(dist < APPLE_COLLECT_RANGE) {
                        appleItems[i].collected = true;
                        applesCollected++;
                    }
                }
                if(applesCollected >= TOTAL_APPLES) {
                    missionActive = false;
                    gameState = GS_MISSION_SUCCESS;
                }
            }

            // Level 2: ambil telur
            if(currentMission == MISSION_TELUR) {
                for(int i = 0; i < TOTAL_EGGS; i++) {
                    if(eggItems[i].collected) continue;
                    float dx = playerX - eggItems[i].x;
                    float dz = playerZ - eggItems[i].z;
                    float dist = sqrtf(dx*dx+dz*dz);
                    if(dist < EGG_COLLECT_RANGE) {
                        eggItems[i].collected = true;
                        eggsCollected++;
                    }
                }
                if(eggsCollected >= TOTAL_EGGS) {
                    missionActive = false;
                    gameState = GS_MISSION_SUCCESS;
                }
            }

            // Level 3: perah sapi
            if(currentMission == MISSION_SUSU) {
                if(nearCow && !isMillking) {
                    isMillking  = 1;
                    milkAnim    = 0.f;
                    udderSwing  = 0.f;
                    cowIsMoving = false;
                }
                if(milkCount >= TARGET_MILK) {
                    missionActive = false;
                    gameState = GS_MISSION_SUCCESS;
                }
            }
        }
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
    if(!treesApple.empty()){
        initColliders();
    }

    float rad=playerYaw*(PI/180.0f);
    bool isWalking=false;

    bool moveForward  = keys['w']||keys['W']||specialKeys[GLUT_KEY_UP];
    bool moveBackward = keys['s']||keys['S']||specialKeys[GLUT_KEY_DOWN];
    bool turnLeft     = keys['a']||keys['A']||specialKeys[GLUT_KEY_LEFT];
    bool turnRight    = keys['d']||keys['D']||specialKeys[GLUT_KEY_RIGHT];

    if(moveForward) {
        float nx=playerX-sinf(rad)*moveSpeed;
        float nz=playerZ-cosf(rad)*moveSpeed;
        if(!checkCollision(nx, playerZ)) playerX=nx;
        if(!checkCollision(playerX, nz)) playerZ=nz;
        isWalking=true;
    }
    if(moveBackward){
        float nx=playerX+sinf(rad)*moveSpeed;
        float nz=playerZ+cosf(rad)*moveSpeed;
        if(!checkCollision(nx, playerZ)) playerX=nx;
        if(!checkCollision(playerX, nz)) playerZ=nz;
        isWalking=true;
    }
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
        udderSwing += 0.033f * 8.f;
        milkAnim   += 0.033f * 6.f;
        if(milkAnim > 3.f){
            milkAnim   = 0.f;
            udderSwing = 0.f;
            isMillking = 0;
            milkCount++;
            cowIsMoving = true;
        }
    }

    {
        float dx = playerX - cowX;
        float dz = playerZ - cowZ;
        float dist = sqrtf(dx*dx + dz*dz);
        nearCow    = (dist < MILK_INTERACT_RANGE);
        milkPrompt = nearCow && !isMillking;
    }

    if(cowIsMoving && !isMillking) {
        updateCowAI();
    }

    // gerbang sapi
    {
        float dx = playerX - GATE_SAPI_X;
        float dz = playerZ - GATE_SAPI_Z;
        nearGateSapi = (sqrtf(dx*dx + dz*dz) < GATE_INTERACT_RANGE);
    }
    // gerbang ayam
    {
        float dx = playerX - GATE_AYAM_X;
        float dz = playerZ - GATE_AYAM_Z;
        nearGateAyam = (sqrtf(dx*dx + dz*dz) < GATE_INTERACT_RANGE);
    }
    // Animasi gerbang
    float gateTarget;
    gateTarget = gateSapiOpen ? 90.0f : 0.0f;
    if(gateSapiAngle < gateTarget) gateSapiAngle += 3.0f;
    if(gateSapiAngle > gateTarget) gateSapiAngle -= 3.0f;
    if(fabsf(gateSapiAngle - gateTarget) < 1.0f) gateSapiAngle = gateTarget;

    gateTarget = gateAyamOpen ? 90.0f : 0.0f;
    if(gateAyamAngle < gateTarget) gateAyamAngle += 3.0f;
    if(gateAyamAngle > gateTarget) gateAyamAngle -= 3.0f;
    if(fabsf(gateAyamAngle - gateTarget) < 1.0f) gateAyamAngle = gateTarget;
    
    // Tisya
	{
    float dx = playerX - tisyaX;
    float dz = playerZ - tisyaZ;
    nearTisya = (sqrtf(dx*dx + dz*dz) < TISYA_TALK_RANGE);
	}
	
	// Rumah Bro
	{
	    float dx = playerX - (8.0f);
	    float dz = playerZ - (-105.0f);
	    nearRumahBro = (!insideRumahBro && sqrtf(dx*dx + dz*dz) < RUMAH_INTERACT_RANGE);
	}
	// Keluar dari rumah
	if(insideRumahBro) {
	    float dx = playerX - RUMAH_INTERIOR_X;
	    float dz = playerZ - RUMAH_DOOR_Z_OUT;
	    if(sqrtf(dx*dx + dz*dz) < RUMAH_DOOR_RANGE && (keys['e'] || keys['E'])) {
	        insideRumahBro = false;
	        playerX = prevPlayerX;
	        playerZ = prevPlayerZ;
	        keys['e'] = keys['E'] = false;
	    }
	}
	// Banner rumah 
	if(showRumahBanner) {
	    rumahBannerTimer -= 0.016f;
	    if(rumahBannerTimer <= 0.0f) {
	        showRumahBanner = false;
	        prevPlayerX = playerX;
	        prevPlayerZ = playerZ;
	        playerX = RUMAH_INTERIOR_X;
	        playerZ = RUMAH_INTERIOR_Z;
	        insideRumahBro = true;
	    }
	}

    updateChickens();
    
    // Countdown timer misi apel, telur, dan susu
	if(missionActive && gameState == GS_PLAYING &&
	   (currentMission == MISSION_APEL || currentMission == MISSION_TELUR || currentMission == MISSION_SUSU)) {
	    timerTicks++;
	    if(timerTicks >= 60) {//tar ganti
	        timerTicks = 0;
	        missionTimer -= 1.0f;
	        if(missionTimer <= 0.0f) {
	            missionTimer  = 0.0f;
	            missionActive = false;
	        }
	    }
	}

    if(showLevelUpBanner) {
	    levelUpBannerTimer -= 0.016f;
	    if(levelUpBannerTimer <= 0.0f) {
	        showLevelUpBanner = false;
	        // Pindah Tisya ke koordinat baru sesuai level
	        if(relationLevel == REL_PACARAN) {
	            tisyaX = 5.0f; tisyaZ = -10.0f; tisyaYaw = 90.0f;
	        } else if(relationLevel == REL_PERTUNANGAN) {
	            tisyaX = -5.0f; tisyaZ = -20.0f; tisyaYaw = 270.0f;
	        } else if(relationLevel == REL_PERNIKAHAN) {
	            tisyaX = 0.0f; tisyaZ = 5.0f; tisyaYaw = 180.0f;
	        }
	    }
	}
    glutPostRedisplay();
    glutTimerFunc(16,update,0);
}


//  Main

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
    generateTextures();
}

int main(int argc,char **argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(1280,720);
    glutCreateWindow("Percobaan 427 kali Gilang mendekati Tisya");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    init();
    initHouses();
    initChickens();

    cowX = cowWaypoints[0].x;
    cowZ = cowWaypoints[0].z;
    cowCurrentWaypoint = 1;
	gameState = GS_PLAYING;
	missionActive = false; 
	initAppleItems();

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
