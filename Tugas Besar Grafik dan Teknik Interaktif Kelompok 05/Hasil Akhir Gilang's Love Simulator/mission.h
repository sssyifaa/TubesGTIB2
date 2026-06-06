#ifndef MISSION_H
#define MISSION_H

// Semua enum, struct, dan variabel ada di global.h
#include "global.h"

// ================================================================
//  Fungsi mission & dialog
// ================================================================

void pushDialog(const char* spk, const char* txt);
void clearDialog();
void showStoryBefore();
void showSuccessDialog();
void showFailDialog();
void showRetryDialog();
void showBroDialog();

void initAppleItems();
void initEggItems();

void drawGroundApple(float x, float y, float z);
void drawAppleItems();
void updateAppleCollection(float px, float pz);
bool allApplesCollected();

void drawEgg(float x, float z);
void drawEggItems();
void updateEggCollection(float px, float pz);
bool allEggsCollected();

#endif
