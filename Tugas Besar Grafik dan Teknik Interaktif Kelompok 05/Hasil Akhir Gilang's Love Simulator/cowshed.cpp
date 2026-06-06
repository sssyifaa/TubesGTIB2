#include "cowshed.h"
#include "global.h"

void updateCowGate(float dt) {
    float target = gateSapiOpen ? -95.0f : 0.0f;
    gateSapiAngle += (target - gateSapiAngle) * 6.0f * dt;
}
