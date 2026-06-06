#include "chikenshed.h"
#include "global.h"

void updateChickenGate(float dt) {
    float target = gateAyamOpen ? -95.0f : 0.0f;
    gateAyamAngle += (target - gateAyamAngle) * 6.0f * dt;
}
