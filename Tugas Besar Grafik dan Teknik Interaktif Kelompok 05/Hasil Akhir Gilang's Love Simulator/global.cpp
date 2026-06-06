#include "global.h"

// ================================================================
//  Player
// ================================================================
float playerX     = 0.0f;
float playerY     = 0.0f;
float playerZ     = 12.0f;
float playerYaw   = 180.0f;
float playerPitch = 0.0f;
float moveSpeed   = 0.15f;
float velocityY   = 0.0f;
bool  isGrounded  = true;
float walkAnim    = 0.0f;
float swingAngle  = 0.0f;
int   playerLives = 3;
bool  keys[256]        = {false};
bool  specialKeys[256] = {false};

// ================================================================
//  NPC Tisya
// ================================================================
float tisyaX        = 0.0f;
float tisyaZ        = 9.0f;
float tisyaYaw      = 180.0f;
float tisyaIdleTime = 0.0f;

// ================================================================
//  NPC Bro
// ================================================================
bool nearBro       = false;
bool broDialogDone = false;

// ================================================================
//  Sapi
// ================================================================
float cowX       = 22.0f;
float cowZ       = -35.0f;
float cowAngle   = 0.0f;
float walkPhase  = 0.0f;
float tailPhase  = 0.0f;
float udderSwing = 0.0f;
int   milkCount  = 0;
int   isMillking = 0;
float milkAnim   = 0.0f;
bool  cowIsMoving= true;
float TONG_X     = 0.0f;
float TONG_Z     = 0.0f;
bool  nearCow    = false;
bool  milkPrompt = false;

// ================================================================
//  Gerbang
// ================================================================
bool  gateSapiOpen  = false;
float gateSapiAngle = 0.0f;
bool  nearGateSapi  = false;
bool  gateAyamOpen  = false;
float gateAyamAngle = 0.0f;
bool  nearGateAyam  = false;

// ================================================================
//  Ember & Rumah Bro
// ================================================================
bool  emberSudahDiambil = false;
float emberInteractRange= 2.5f;
bool  nearEmber         = false;
bool  insideRumahBro    = false;
bool  nearRumahBro      = false;
float prevPlayerX       = 0.0f;
float prevPlayerZ       = 0.0f;
bool  showRumahBanner   = false;
float rumahBannerTimer  = 0.0f;

// ================================================================
//  Ayam
// ================================================================
Chicken chickens[NUM_CHICKENS];

// ================================================================
//  Pohon & Rumah
// ================================================================
std::vector<TreePos> treesApple;
std::vector<House>   houses;

// ================================================================
//  Mission / Dialog
// ================================================================
GameState     gameState      = GS_PLAYING; // main() akan override, tapi konsisten
MissionID     currentMission = MISSION_APEL;
RelationLevel relationLevel  = REL_STRANGER;
bool          missionActive  = false;
bool          missionJustDone = false;
float         missionTimer   = 60.0f;
int           timerTicks     = 0;

bool  showLevelUpBanner  = false;
float levelUpBannerTimer = 0.0f;

bool  nearTisya    = false;
bool  dialogActive = false;
int   dialogCount  = 0;
int   dialogIndex  = 0;
DialogLine dialogLines[MAX_DIALOG_LINES];

AppleItem appleItems[TOTAL_APPLES];
bool      applesInitialized = false;
int       applesCollected   = 0;

EggItem eggItems[TOTAL_EGGS];
bool    eggsInitialized = false;
int     eggsCollected   = 0;

float heartAnim = 0.0f;
