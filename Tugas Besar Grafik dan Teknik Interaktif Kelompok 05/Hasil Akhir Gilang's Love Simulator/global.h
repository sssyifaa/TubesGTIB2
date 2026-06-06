#ifndef GLOBAL_H
#define GLOBAL_H

#include <vector>

// ================================================================
//  Struct
// ================================================================

struct Color3 {
    float r, g, b;
};

// Sesuai dengan mainApanih: House pakai cx,cz,width,depth,wallH,roofH,yardD
struct House {
    float cx, cz, width, depth, wallH, roofH, yardD;
    int   side, type;
    Color3 wall, roof, door, win;
};

struct Chicken {
    float x, z;
    float angle;
    float legAnim;
    float speed;
    int   turnTimer;
    float turnTarget;
};

struct TreePos { float x, z; };

struct AppleItem {
    float x, y, z;
    bool  collected;
};

struct EggItem {
    float x, z;
    bool  collected;
};

struct DialogLine {
    const char* speaker;
    const char* text;
};

// ================================================================
//  Enum  (didefinisikan HANYA di sini, tidak di mission.h)
// ================================================================

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
    MISSION_APEL,
    MISSION_TELUR,
    MISSION_SUSU
};

enum RelationLevel {
    REL_STRANGER = 0,
    REL_PACARAN,
    REL_PERTUNANGAN,
    REL_PERNIKAHAN
};

// ================================================================
//  Konstanta global
// ================================================================

static const int   NUM_CHICKENS  = 5;
static const float CHICKEN_CX    = 32.0f;
static const float CHICKEN_CZ    = -35.0f;
static const float BRO_X         = 4.0f;
static const float BRO_Z         = -33.0f;

static const float RUMAH_INTERIOR_X  = 500.0f;
static const float RUMAH_INTERIOR_Z  = 500.0f;
static const float RUMAH_DOOR_Z_OUT  = 507.5f;
static const float RUMAH_INTERACT_RANGE = 5.5f;
static const float RUMAH_DOOR_RANGE     = 2.0f;

static const float TISYA_TALK_RANGE = 1.5f;
static const float BRO_TALK_RANGE   = 2.0f;

static const float GATE_INTERACT_RANGE = 2.5f;
static const float GATE_SAPI_X  = 22.0f;
static const float GATE_SAPI_Z  = -31.0f;
static const float GATE_AYAM_X  = 32.0f;
static const float GATE_AYAM_Z  = -31.0f;

static const float MILK_INTERACT_RANGE = 3.0f;

static const int   TOTAL_APPLES       = 10;
static const int   TOTAL_EGGS         = 8;
static const int   TARGET_MILK        = 5;
static const float APPLE_COLLECT_RANGE= 1.8f;
static const float EGG_COLLECT_RANGE  = 1.5f;

static const int   MAX_DIALOG_LINES   = 12;

// ================================================================
//  Player
// ================================================================

extern float playerX, playerY, playerZ;
extern float playerYaw, playerPitch;
extern float moveSpeed;
extern float velocityY;
extern bool  isGrounded;
extern float walkAnim;
extern float swingAngle;
extern int   playerLives;
extern bool  keys[256];
extern bool  specialKeys[256];

// ================================================================
//  NPC Tisya
// ================================================================

extern float tisyaX, tisyaZ, tisyaYaw;
extern float tisyaIdleTime;

// ================================================================
//  NPC Bro
// ================================================================

extern bool nearBro;
extern bool broDialogDone;

// ================================================================
//  Sapi
// ================================================================

extern float cowX, cowZ, cowAngle;
extern float walkPhase, tailPhase, udderSwing;
extern int   milkCount;
extern int   isMillking;
extern float milkAnim;
extern bool  cowIsMoving;
extern float TONG_X, TONG_Z;
extern bool  nearCow;
extern bool  milkPrompt;

// ================================================================
//  Gerbang
// ================================================================

extern bool  gateSapiOpen;
extern float gateSapiAngle;
extern bool  nearGateSapi;
extern bool  gateAyamOpen;
extern float gateAyamAngle;
extern bool  nearGateAyam;

// ================================================================
//  Ember & Rumah Bro
// ================================================================

extern bool  emberSudahDiambil;
extern float emberInteractRange;
extern bool  nearEmber;
extern bool  insideRumahBro;
extern bool  nearRumahBro;
extern float prevPlayerX, prevPlayerZ;
extern bool  showRumahBanner;
extern float rumahBannerTimer;

// ================================================================
//  Ayam
// ================================================================

extern Chicken chickens[NUM_CHICKENS];

// ================================================================
//  Pohon & Rumah
// ================================================================

extern std::vector<TreePos> treesApple;
extern std::vector<House>   houses;

// ================================================================
//  Mission / Dialog
// ================================================================

extern GameState     gameState;
extern MissionID     currentMission;
extern RelationLevel relationLevel;
extern bool          missionActive;
extern bool          missionJustDone; // flag: misi selesai, tunggu player dekati Tisya
extern float         missionTimer;
extern int           timerTicks;
extern int           playerLives;

extern bool  showLevelUpBanner;
extern float levelUpBannerTimer;

extern bool  nearTisya;
extern bool  dialogActive;
extern int   dialogCount;
extern int   dialogIndex;
extern DialogLine dialogLines[MAX_DIALOG_LINES];

// Item apel & telur
extern AppleItem appleItems[TOTAL_APPLES];
extern bool      applesInitialized;
extern int       applesCollected;

extern EggItem eggItems[TOTAL_EGGS];
extern bool    eggsInitialized;
extern int     eggsCollected;

extern float heartAnim;

#endif

