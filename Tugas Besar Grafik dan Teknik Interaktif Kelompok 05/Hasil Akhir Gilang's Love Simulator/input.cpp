#include "input.h"
#include "global.h"
#include "mission.h"
#include "collision.h"
#include <GL/glut.h>
#include <cstdlib>

// ================================================================
//  Keyboard down
// ================================================================
void keyboard(unsigned char key, int x, int y) {
    if (key == 27) exit(0); // ESC keluar
    if (key < 256) keys[key] = true;

    // ---- Tekan E / Enter ----
    if (key == 'e' || key == 'E' || key == '\r' || key == '\n') {

        // Lanjut dialog
        if (dialogActive) {
            dialogIndex++;
            if (dialogIndex >= dialogCount) {
                // Dialog habis
                dialogActive = false;
                dialogCount  = 0;
                dialogIndex  = 0;

                if (gameState == GS_STORY) {
                    // Story intro selesai -> mulai misi
                    gameState     = GS_PLAYING;
                    missionActive = true;
                    missionTimer  = 60.0f;
                    timerTicks    = 0;

                } else if (gameState == GS_MISSION_SUCCESS) {
                    // Misi berhasil -> naik level
                    if (currentMission == MISSION_APEL) {
                        relationLevel  = REL_PACARAN;
                        currentMission = MISSION_TELUR;
                        // Reset egg
                        eggsCollected    = 0;
                        eggsInitialized  = false;
                        initEggItems();
                    } else if (currentMission == MISSION_TELUR) {
                        relationLevel  = REL_PERTUNANGAN;
                        currentMission = MISSION_SUSU;
                        milkCount      = 0;
                    } else if (currentMission == MISSION_SUSU) {
                        relationLevel = REL_PERNIKAHAN;
                        gameState     = GS_WIN;
                        missionActive = false;
                        return;
                    }
                    // Tampilkan banner, lalu lanjut playing
                    showLevelUpBanner  = true;
                    levelUpBannerTimer = 2.5f;
                    gameState          = GS_PLAYING;
                    missionActive      = true;
                    missionTimer       = 60.0f;
                    timerTicks         = 0;

                } else if (gameState == GS_MISSION_FAIL) {
                    playerLives--;
                    if (playerLives <= 0) {
                        gameState     = GS_GAME_OVER;
                        missionActive = false;
                    } else {
                        // Retry: reset item dan timer
                        if (currentMission == MISSION_APEL) {
                            applesCollected   = 0;
                            applesInitialized = false;
                            initAppleItems();
                        } else if (currentMission == MISSION_TELUR) {
                            eggsCollected    = 0;
                            eggsInitialized  = false;
                            initEggItems();
                        } else if (currentMission == MISSION_SUSU) {
                            milkCount = 0;
                        }
                        missionTimer  = 60.0f;
                        timerTicks    = 0;
                        gameState     = GS_PLAYING;
                        missionActive = true;
                    }
                }
            }
            return; // sudah ditangani, tidak perlu proses interaksi lain
        }

        // ---- Interaksi dunia (hanya saat GS_PLAYING dan tidak dialog) ----
        if (gameState == GS_PLAYING) {

            // Bicara Tisya
            if (nearTisya) {
                showStoryBefore();
                return;
            }
            // Bicara Bro
            if (nearBro && !broDialogDone) {
                showBroDialog();
                broDialogDone = true;
                return;
            }
            // Masuk rumah Bro
            if (nearRumahBro && !insideRumahBro && emberSudahDiambil == false) {
                // Bisa masuk kalau sudah ngobrol bro
                if (broDialogDone) {
                    showRumahBanner  = true;
                    rumahBannerTimer = 1.5f;
                }
                return;
            }
            // Ambil ember
            if (insideRumahBro && nearEmber && !emberSudahDiambil) {
                emberSudahDiambil = true;
                return;
            }
            // Buka/tutup gerbang sapi
            if (nearGateSapi) {
                gateSapiOpen = !gateSapiOpen;
                initColliders();
                return;
            }
            // Buka/tutup gerbang ayam
            if (nearGateAyam) {
                gateAyamOpen = !gateAyamOpen;
                initColliders();
                return;
            }
        }
    }

    // ---- Tekan F: perah susu ----
    if ((key == 'f' || key == 'F') &&
        milkPrompt && !isMillking &&
        currentMission == MISSION_SUSU &&
        missionActive && gameState == GS_PLAYING) {
        isMillking = 1;
    }
}

// ================================================================
//  Keyboard up
// ================================================================
void keyboardUp(unsigned char key, int x, int y) {
    if (key < 256) keys[key] = false;
}

// ================================================================
//  Special keys (arrow + lainnya)
// ================================================================
void specialKey(int key, int x, int y) {
    if (key < 256) specialKeys[key] = true;
}

void specialKeyUp(int key, int x, int y) {
    if (key < 256) specialKeys[key] = false;
}

// ================================================================
//  Mouse motion (kamera ditangani di main via passiveMotion)
// ================================================================
void mouseMotion(int x, int y) {}
