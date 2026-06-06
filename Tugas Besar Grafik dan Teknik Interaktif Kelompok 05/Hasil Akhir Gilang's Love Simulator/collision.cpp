#include "collision.h"
#include "global.h"
#include <cmath>

std::vector<AABB> colliders;

static const float PLAYER_RADIUS = 0.4f;

// addBox pakai cx,cz,width,depth seperti mainApanih
void addBox(float cx, float cz, float w, float d) {
    AABB b;
    b.minX = cx - w * 0.5f;
    b.maxX = cx + w * 0.5f;
    b.minZ = cz - d * 0.5f;
    b.maxZ = cz + d * 0.5f;
    colliders.push_back(b);
}

void initColliders() {
    colliders.clear();

    // Rumah-rumah (std::vector<House> dari global)
    for(int i = 0; i < (int)houses.size(); i++) {
        const House &h = houses[i];
        addBox(h.cx, h.cz, h.width + 0.3f, h.depth + 0.3f);
    }

    // PAGAR + kandang sapi
    addBox(22.0f,  -39.0f, 8.5f, 0.4f);
    addBox(18.0f,  -35.0f, 0.4f, 8.5f);
    addBox(26.0f,  -35.0f, 0.4f, 8.5f);
    addBox(19.5f,  -31.0f, 3.5f, 0.4f);
    addBox(24.5f,  -31.0f, 3.5f, 0.4f);
    if(!gateSapiOpen) addBox(22.0f, -31.0f, 2.0f, 0.4f);

    // PAGAR kandang ayam
    addBox(32.0f,  -39.0f, 8.5f, 0.4f);
    addBox(28.0f,  -35.0f, 0.4f, 8.5f);
    addBox(36.0f,  -35.0f, 0.4f, 8.5f);
    addBox(29.5f,  -31.0f, 3.5f, 0.4f);
    addBox(34.5f,  -31.0f, 3.5f, 0.4f);
    if(!gateAyamOpen) addBox(32.0f, -31.0f, 2.0f, 0.4f);

    // POHON APEL
    for(int i = 0; i < (int)treesApple.size(); i++)
        addBox(treesApple[i].x, treesApple[i].z, 0.9f, 0.9f);

    // TISYA
    addBox(tisyaX, tisyaZ, 1.0f, 1.0f);

    // BRO NPC
    addBox(BRO_X, BRO_Z, 1.0f, 1.0f);

    // Dalam rumah Bro
    if(insideRumahBro) {
        float rx = RUMAH_INTERIOR_X, rz = RUMAH_INTERIOR_Z;
        float RW = 20.0f, RD = 20.0f;
        addBox(rx,          rz - RD/2, RW,   0.5f);
        addBox(rx,          rz + RD/2, RW,   0.5f);
        addBox(rx - RW/2,   rz,        0.5f, RD);
        addBox(rx + RW/2,   rz,        0.5f, RD);
        addBox(rx - RW/2 + 3.5f, rz - RD/2 + 3.5f, 2.0f, 1.2f);
    }
}

bool checkCollision(float nx, float nz) {
    for(int i = 0; i < (int)colliders.size(); i++) {
        if(nx + PLAYER_RADIUS > colliders[i].minX &&
           nx - PLAYER_RADIUS < colliders[i].maxX &&
           nz + PLAYER_RADIUS > colliders[i].minZ &&
           nz - PLAYER_RADIUS < colliders[i].maxZ)
            return true;
    }
    return false;
}
