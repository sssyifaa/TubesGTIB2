#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include "global.h"
#include "utils.h"
#include "texture.h"
#include "collision.h"
#include "camera.h"
#include "player.h"
#include "tisya.h"
#include "bro.h"
#include "mission.h"
#include "environment.h"
#include "house.h"
#include "cow.h"
#include "chiken.h"
#include "cowshed.h"
#include "chikenshed.h"
#include "shadow.h"
#include "input.h"

static const float PI  = 3.14159265f;
static int screenW = 1280, screenH = 720;

// ================================================================
//  HUD helpers
// ================================================================
static void drawText2D(float x, float y, const char* str) {
    glRasterPos2f(x, y);
    for (const char* c = str; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}
static void drawText2DSmall(float x, float y, const char* str) {
    glRasterPos2f(x, y);
    for (const char* c = str; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
}

// ================================================================
//  HUD
// ================================================================
static void drawHUD() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    glOrtho(0, screenW, 0, screenH, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();

    char buf[128];

    // Lives (hati sederhana pakai O)
    glColor3f(1,0.2f,0.2f);
    for (int i = 0; i < playerLives; i++) {
        snprintf(buf, sizeof(buf), "?");
        drawText2D(10.0f + i*22.0f, screenH - 28.0f, "O");
    }

    // Status relasi
    const char* relStr[] = {"Stranger","Pacaran","Pertunangan","Menikah"};
    glColor3f(1.0f,0.9f,0.3f);
    snprintf(buf, sizeof(buf), "Status: %s", relStr[relationLevel]);
    drawText2D(10, screenH - 52, buf);

    // Timer misi
    if (missionActive &&
        (currentMission == MISSION_APEL ||
         currentMission == MISSION_TELUR ||
         currentMission == MISSION_SUSU)) {
        if (missionTimer <= 10.0f) glColor3f(1,0.2f,0.2f);
        else                       glColor3f(1,1,0.3f);
        snprintf(buf, sizeof(buf), "Waktu: %.0f", missionTimer);
        drawText2D(screenW/2 - 40, screenH - 30, buf);
    }

    // Progress misi
    if (missionActive) {
        if (currentMission == MISSION_APEL) {
            snprintf(buf, sizeof(buf), "Apel: %d / %d", applesCollected, TOTAL_APPLES);
            glColor3f(0.9f,1,0.4f);
            drawText2D(10, screenH - 75, buf);
        } else if (currentMission == MISSION_TELUR) {
            snprintf(buf, sizeof(buf), "Telur: %d / %d", eggsCollected, TOTAL_EGGS);
            glColor3f(0.9f,1,0.4f);
            drawText2D(10, screenH - 75, buf);
            if (!emberSudahDiambil) {
                glColor3f(1.0f,0.6f,0.2f);
                drawText2DSmall(10, screenH - 98, "Ambil ember dulu di rumah Bro!");
            }
        } else if (currentMission == MISSION_SUSU) {
            snprintf(buf, sizeof(buf), "Susu: %d / %d", milkCount, TARGET_MILK);
            glColor3f(0.9f,1,0.4f);
            drawText2D(10, screenH - 75, buf);
            if (milkPrompt && !isMillking) {
                glColor3f(1,1,1);
                drawText2DSmall(screenW/2-70, 80, "[F] Perah susu");
            }
        }
    }

    // Prompt interaksi (hanya saat tidak dialog dan sedang playing)
    if (!dialogActive && gameState == GS_PLAYING) {
        glColor3f(1,1,0.7f);
        // Misi selesai: arahkan player ke Tisya
        if (missionJustDone && !nearTisya) {
            glColor3f(0.3f,1.0f,0.4f);
            drawText2DSmall(screenW/2-90, 60, "Temui Tisya!");
        }
        // Belum mulai misi -> arahkan ke Tisya
        if (!missionActive && !missionJustDone && !dialogActive) {
            glColor3f(1.0f,1.0f,0.5f);
            drawText2DSmall(screenW/2-110, 60, "Temui Tisya untuk mulai misi!");
        }
        if (nearTisya)
            drawText2DSmall(screenW/2-90, 60, "[E] Bicara dengan Tisya");
        if (nearBro && !broDialogDone)
            drawText2DSmall(screenW/2-80, 60, "[E] Bicara dengan Bro");
        if (nearGateSapi)
            drawText2DSmall(screenW/2-80, 40, "[E] Buka/Tutup Gerbang Sapi");
        if (nearGateAyam)
            drawText2DSmall(screenW/2-80, 40, "[E] Buka/Tutup Gerbang Ayam");
        if (nearRumahBro && !insideRumahBro)
            drawText2DSmall(screenW/2-70, 40, "[E] Masuk Rumah Bro");
        if (insideRumahBro)
            drawText2DSmall(screenW/2-60, 40, "[E] Keluar Rumah");
        if (nearEmber && !emberSudahDiambil)
            drawText2DSmall(screenW/2-50, 20, "[E] Ambil Ember");
    }

    // Banner level up (hanya teks, tidak lock input)
    if (showLevelUpBanner) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0,0,0,0.55f);
        glBegin(GL_QUADS);
            glVertex2f(screenW/2-220, screenH/2-35);
            glVertex2f(screenW/2+220, screenH/2-35);
            glVertex2f(screenW/2+220, screenH/2+35);
            glVertex2f(screenW/2-220, screenH/2+35);
        glEnd();
        glDisable(GL_BLEND);
        glColor3f(1.0f,0.9f,0.2f);
        const char* lvlMsg[] = {"","Kamu Pacaran!","Kalian Bertunangan!","Selamat Menikah!"};
        drawText2D(screenW/2-90, screenH/2-8, lvlMsg[relationLevel]);
    }

    // Banner masuk rumah (layar hitam fade)
    if (showRumahBanner) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // fade makin gelap saat timer makin kecil
        float alpha = 1.0f - (rumahBannerTimer / 1.5f);
        glColor4f(0,0,0, clampF(alpha, 0.0f, 1.0f));
        glBegin(GL_QUADS);
            glVertex2f(0,0); glVertex2f(screenW,0);
            glVertex2f(screenW,screenH); glVertex2f(0,screenH);
        glEnd();
        glDisable(GL_BLEND);
        glColor3f(1,1,1);
        drawText2DSmall(screenW/2-80, screenH/2, "Masuk ke Rumah Bro...");
    }

    // Dialog box
    if (dialogActive && dialogIndex < dialogCount) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0,0,0,0.75f);
        glBegin(GL_QUADS);
            glVertex2f(30, 20); glVertex2f(screenW-30, 20);
            glVertex2f(screenW-30, 130); glVertex2f(30, 130);
        glEnd();
        glDisable(GL_BLEND);

        const char* spk = dialogLines[dialogIndex].speaker;
        const char* txt = dialogLines[dialogIndex].text;

        glColor3f(1.0f,0.85f,0.3f);
        drawText2D(50, 100, spk);
        glColor3f(1,1,1);
        drawText2DSmall(50, 75, txt);
        glColor3f(0.6f,0.6f,0.6f);
        drawText2DSmall(screenW-220, 30, "[E / Enter] Lanjut");
    }

    // Game Over
    if (gameState == GS_GAME_OVER) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0,0,0,0.82f);
        glBegin(GL_QUADS);
            glVertex2f(0,0); glVertex2f(screenW,0);
            glVertex2f(screenW,screenH); glVertex2f(0,screenH);
        glEnd();
        glDisable(GL_BLEND);
        glColor3f(1,0.2f,0.2f);
        drawText2D(screenW/2-80, screenH/2+20, "GAME OVER");
        glColor3f(1,1,1);
        drawText2DSmall(screenW/2-100, screenH/2-20, "Gilang kehabisan kesempatan...");
    }

    // Win
    if (gameState == GS_WIN) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.1f,0.0f,0.2f,0.88f);
        glBegin(GL_QUADS);
            glVertex2f(0,0); glVertex2f(screenW,0);
            glVertex2f(screenW,screenH); glVertex2f(0,screenH);
        glEnd();
        glDisable(GL_BLEND);
        glColor3f(1.0f,0.9f,0.2f);
        drawText2D(screenW/2-150, screenH/2+30, "Selamat! Gilang & Tisya Menikah!");
        glColor3f(1,1,1);
        drawText2DSmall(screenW/2-90, screenH/2-10, "Terima kasih telah bermain!");
    }

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ================================================================
//  Display
// ================================================================
static void display() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (insideRumahBro) {
        glClearColor(0.10f, 0.08f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        setupInteriorLighting();
        updateCamera();
        drawHouseInside();
        drawPlayer(); // FIX: player (Gilang) ikut masuk ke dalam rumah
    } else {
        glClearColor(0.72f, 0.77f, 0.83f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        setupEnvironment();
        updateCamera();
        drawScene();
    }

    drawHUD();
    glutSwapBuffers();
}

// ================================================================
//  Reshape
// ================================================================
static void reshape(int w, int h) {
    screenW = w; screenH = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w/(double)h, 0.1, 500.0);
    glMatrixMode(GL_MODELVIEW);
}

// ================================================================
//  Mouse
// ================================================================
static void mouseBtn(int btn, int state, int x, int y) {}

static void passiveMotion(int x, int y) {
    handleCamLook(x, y, screenW, screenH);
}

// ================================================================
//  Update
// ================================================================
static void update(int value) {
    const float dt = 0.016f;

    // Selalu update animasi dasar
    tailPhase    += dt * 2.5f;
    tisyaIdleTime += dt;
    heartAnim    += dt;

    // Gerbang selalu animasi
    updateCowGate(dt);
    updateChickenGate(dt);

    // Hanya update gameplay saat playing
    if (gameState == GS_PLAYING) {

        // Rebuild colliders
        initColliders();

        // Gerak player
        updatePlayer();

        // Sapi
        if (isMillking) {
            udderSwing += dt * 8.0f;
            milkAnim   += dt * 6.0f;
            if (milkAnim > 3.0f) {
                milkAnim   = 0.0f;
                udderSwing = 0.0f;
                isMillking = 0;
                milkCount++;
                cowIsMoving = true;
            }
        }
        if (cowIsMoving && !isMillking) updateCowAI();

        // Ayam
        updateChickens();

        // ---- Proximity checks ----

        // Near cow
        {
            float dx = playerX - cowX, dz = playerZ - cowZ;
            nearCow    = (sqrtf(dx*dx+dz*dz) < MILK_INTERACT_RANGE);
            milkPrompt = nearCow && !isMillking;
        }
        // Near gerbang sapi
        {
            float dx = playerX - GATE_SAPI_X, dz = playerZ - GATE_SAPI_Z;
            nearGateSapi = (sqrtf(dx*dx+dz*dz) < GATE_INTERACT_RANGE);
        }
        // Near gerbang ayam
        {
            float dx = playerX - GATE_AYAM_X, dz = playerZ - GATE_AYAM_Z;
            nearGateAyam = (sqrtf(dx*dx+dz*dz) < GATE_INTERACT_RANGE);
        }
        // Near Tisya
        {
            float dx = playerX - tisyaX, dz = playerZ - tisyaZ;
            nearTisya = (sqrtf(dx*dx+dz*dz) < TISYA_TALK_RANGE);
            // Misi selesai + player sudah dekat Tisya -> tampilkan dialog sukses
            if (missionJustDone && nearTisya && !dialogActive) {
                missionJustDone = false;
                showSuccessDialog();
            }
        }
        // Near Bro
        {
            float dx = playerX - BRO_X, dz = playerZ - BRO_Z;
            nearBro = (sqrtf(dx*dx+dz*dz) < BRO_TALK_RANGE);
        }
        // Near rumah Bro (dari luar)
        {
            float dx = playerX - 8.0f, dz = playerZ - (-105.0f);
            nearRumahBro = (!insideRumahBro && sqrtf(dx*dx+dz*dz) < RUMAH_INTERACT_RANGE);
        }
        // Near ember (di dalam rumah)
        if (insideRumahBro && !emberSudahDiambil) {
            float ex = RUMAH_INTERIOR_X - 10.0f + 3.5f;
            float ez = RUMAH_INTERIOR_Z - 10.0f + 3.5f;
            float dx = playerX - ex, dz = playerZ - ez;
            nearEmber = (sqrtf(dx*dx+dz*dz) < emberInteractRange);
        } else {
            nearEmber = false;
        }

        // Keluar rumah Bro (E di dekat pintu)
        if (insideRumahBro) {
            float dx = playerX - RUMAH_INTERIOR_X;
            float dz = playerZ - RUMAH_DOOR_Z_OUT;
            if (sqrtf(dx*dx+dz*dz) < RUMAH_DOOR_RANGE && (keys['e'] || keys['E'])) {
                insideRumahBro = false;
                playerX = prevPlayerX;
                playerZ = prevPlayerZ;
                keys['e'] = keys['E'] = false;
                initColliders();
            }
        }

        // Koleksi apel
        if (currentMission == MISSION_APEL && missionActive && applesInitialized)
            updateAppleCollection(playerX, playerZ);

        // Koleksi telur — hanya bisa kalau ember sudah diambil
        if (currentMission == MISSION_TELUR && missionActive && eggsInitialized && emberSudahDiambil)
            updateEggCollection(playerX, playerZ);

        // Cek win condition misi
        if (missionActive) {
            bool done = false;
            if (currentMission == MISSION_APEL  && allApplesCollected())      done = true;
            if (currentMission == MISSION_TELUR && allEggsCollected())         done = true;
            if (currentMission == MISSION_SUSU  && milkCount >= TARGET_MILK)  done = true;
            if (done && !missionJustDone) {
                missionJustDone = true;
                missionActive   = false; // stop timer
            }
        }

        // Timer misi countdown
        if (missionActive &&
            (currentMission == MISSION_APEL ||
             currentMission == MISSION_TELUR ||
             currentMission == MISSION_SUSU)) {
            timerTicks++;
            if (timerTicks >= 60) {
                timerTicks = 0;
                missionTimer -= 1.0f;
                if (missionTimer <= 0.0f) {
                    missionTimer  = 0.0f;
                    missionActive = false;
                    showFailDialog();
                }
            }
        }
    } // end GS_PLAYING

    // Banner rumah (jalan di semua state)
    if (showRumahBanner) {
        rumahBannerTimer -= dt;
        if (rumahBannerTimer <= 0.0f) {
            showRumahBanner = false;
            prevPlayerX    = playerX;
            prevPlayerZ    = playerZ;
            playerX        = RUMAH_INTERIOR_X;
            playerZ        = RUMAH_INTERIOR_Z;
            insideRumahBro = true;
            initColliders();
        }
    }

    // Banner level up timer (hanya display, tidak lock gameplay)
    if (showLevelUpBanner) {
        levelUpBannerTimer -= dt;
        if (levelUpBannerTimer <= 0.0f) {
            showLevelUpBanner = false;
            // Pindah posisi Tisya sesuai level
            if (relationLevel == REL_PACARAN)
                { tisyaX = 5.0f;  tisyaZ = -10.0f; tisyaYaw = 90.0f; }
            else if (relationLevel == REL_PERTUNANGAN)
                { tisyaX = -5.0f; tisyaZ = -20.0f; tisyaYaw = 270.0f; }
            else if (relationLevel == REL_PERNIKAHAN)
                { tisyaX = 0.0f;  tisyaZ = 5.0f;   tisyaYaw = 180.0f; }
            initColliders();
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ================================================================
//  Init OpenGL
// ================================================================
static void initGL() {
    glClearColor(0.72f, 0.77f, 0.83f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    generateTextures();
}

// ================================================================
//  Main
// ================================================================
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenW, screenH);
    glutCreateWindow("Percobaan 1147 kali Gilang mendekati Tisya");

    initGL();
    initHouses();
    initColliders();
    initChickens();
    initCow();
    initAppleItems();
    initEggItems();

    // Mulai GS_PLAYING tapi missionActive false dulu — timer baru jalan setelah ngobrol Tisya
    gameState     = GS_PLAYING;
    missionActive = false;
    missionTimer  = 60.0f;
    timerTicks    = 0;
    dialogActive  = false;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);
    glutMouseFunc(mouseBtn);
    glutPassiveMotionFunc(passiveMotion);
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}
