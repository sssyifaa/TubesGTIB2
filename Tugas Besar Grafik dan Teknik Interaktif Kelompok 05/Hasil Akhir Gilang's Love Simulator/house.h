#ifndef HOUSE_H
#define HOUSE_H

// Fungsi rumah eksterior dan interior
void initHouses();
void drawHouse(const struct House &h);
void drawHouses();
void drawRumahInterior();
void drawHouseInside();  // alias drawRumahInterior

// Helper drawing (dipakai cowshed, chikenshed, environment)
void drawFence(float x0, float z0, float x1, float z1, int posts);
void drawGate(float hinge_x, float hinge_z, float gateAngleDeg,
              float gateWidth, bool isOpenLeft);
void drawKandangSapi(float cx, float cz);
void drawKandangAyam(float cx, float cz);

#endif

