#include "mission.h"
#include <GL/glut.h>
#include <cstdio>
#include <cstring>
#include <cmath>

// ================================================================
//  Dialog
// ================================================================

void pushDialog(const char* spk, const char* txt) {
    if(dialogCount < MAX_DIALOG_LINES) {
        dialogLines[dialogCount].speaker = spk;
        dialogLines[dialogCount].text    = txt;
        dialogCount++;
    }
}

void clearDialog() { dialogCount = 0; dialogIndex = 0; dialogActive = false; }

void showStoryBefore() {
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
        pushDialog("Narator", "Cari Orang lain untuk mengetahui informasi mengenai telur!");
    } else if(currentMission == MISSION_SUSU) {
        pushDialog("Gilang",  "Tisya, aku mau lanjut ke jenjang yang lebih serius dengan kamu.");
        pushDialog("Tisya",   "Kamu harus menunjukan usaha kamu untuk sampai ke titik itu.");
        pushDialog("Gilang",  "Aku siap!");
        pushDialog("Narator", "Pergi ke peternakan dan perah susu dari sapi yang terus bergerak, sebanyak 5 liter!");
    }
    dialogActive = true;
    gameState    = GS_STORY;
}

void showSuccessDialog() {
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
    dialogActive  = true;
    gameState     = GS_MISSION_SUCCESS;
    missionActive = false;
}

void showFailDialog() {
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
    gameState    = GS_MISSION_FAIL;
}

void showRetryDialog() {
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
    gameState    = GS_STORY;
}

void showBroDialog() {
    clearDialog();
    pushDialog("Gilang",  "woy brok");
    pushDialog("Bro",     "kenapa brok");
    pushDialog("Gilang",  "lu ada ember ga brok?");
    pushDialog("Bro",     "ada brok");
    pushDialog("Bro",     "ambil aja di rumah gua brok");
    pushDialog("Gilang",  "lah rumahlu dimane brok?");
    pushDialog("Bro",     "itu di ujung yang sebrangnya ga ada rumah lain");
    pushDialog("Gilang",  "oke brok, thanks yak");
    pushDialog("Bro",     "aman aja brok");
    pushDialog("Narator", "mereka ga kenal satu sama lain");
    dialogActive = true;
    gameState    = GS_STORY;
}

// ================================================================
//  Init & draw apel
// ================================================================

void initAppleItems() {
    float positions[TOTAL_APPLES][2] = {
        {-15.5f,-22.0f},{-20.0f,-24.0f},{-25.5f,-26.0f},
        {-15.0f,-28.0f},{-30.0f,-22.0f},{-35.0f,-26.0f},
        {-20.5f,-32.0f},{-25.0f,-34.0f},{-30.5f,-32.0f},
        {-35.0f,-34.0f}
    };
    applesCollected = 0;
    for(int i = 0; i < TOTAL_APPLES; i++) {
        appleItems[i].x         = positions[i][0];
        appleItems[i].y         = 1.2f;
        appleItems[i].z         = positions[i][1];
        appleItems[i].collected = false;
    }
    applesInitialized = true;
}

void drawGroundApple(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(0.85f, 0.10f, 0.10f);
    glutSolidSphere(0.18f, 8, 8);
    glColor3f(0.20f, 0.55f, 0.10f);
    glTranslatef(0, 0.18f, 0);
    glScalef(0.04f, 0.12f, 0.04f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void drawAppleItems() {
    if(!applesInitialized) return;
    for(int i = 0; i < TOTAL_APPLES; i++) {
        if(!appleItems[i].collected)
            drawGroundApple(appleItems[i].x, appleItems[i].y, appleItems[i].z);
    }
}

void updateAppleCollection(float px, float pz) {
    for(int i = 0; i < TOTAL_APPLES; i++) {
        if(appleItems[i].collected) continue;
        float dx = px - appleItems[i].x;
        float dz = pz - appleItems[i].z;
        if(sqrtf(dx*dx + dz*dz) < APPLE_COLLECT_RANGE) {
            appleItems[i].collected = true;
            applesCollected++;
        }
    }
}

bool allApplesCollected() { return applesCollected >= TOTAL_APPLES; }

// ================================================================
//  Init & draw telur
// ================================================================

void initEggItems() {
    float offX[TOTAL_EGGS] = {-2.5f, 2.5f,-1.0f, 1.0f,-2.5f, 2.5f, 0.0f,-1.5f};
    float offZ[TOTAL_EGGS] = {-2.5f,-2.0f,-1.0f, 2.0f, 2.5f, 2.5f,-2.0f, 1.5f};
    eggsCollected = 0;
    for(int i = 0; i < TOTAL_EGGS; i++) {
        eggItems[i].x         = CHICKEN_CX + offX[i];
        eggItems[i].z         = CHICKEN_CZ + offZ[i];
        eggItems[i].collected = false;
    }
    eggsInitialized = true;
}

void drawEgg(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.12f, z);
    glColor3f(0.95f, 0.92f, 0.85f);
    glScalef(0.8f, 1.0f, 0.8f);
    glutSolidSphere(0.12f, 8, 8);
    glPopMatrix();
}

void drawEggItems() {
    if(!eggsInitialized) return;
    for(int i = 0; i < TOTAL_EGGS; i++) {
        if(!eggItems[i].collected)
            drawEgg(eggItems[i].x, eggItems[i].z);
    }
}

void updateEggCollection(float px, float pz) {
    for(int i = 0; i < TOTAL_EGGS; i++) {
        if(eggItems[i].collected) continue;
        float dx = px - eggItems[i].x;
        float dz = pz - eggItems[i].z;
        if(sqrtf(dx*dx + dz*dz) < EGG_COLLECT_RANGE) {
            eggItems[i].collected = true;
            eggsCollected++;
        }
    }
}

bool allEggsCollected() { return eggsCollected >= TOTAL_EGGS; }
