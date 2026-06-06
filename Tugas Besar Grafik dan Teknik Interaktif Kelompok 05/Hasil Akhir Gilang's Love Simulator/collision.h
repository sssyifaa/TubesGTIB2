#ifndef COLLISION_H
#define COLLISION_H

#include <vector>

struct AABB {
    float minX, maxX, minZ, maxZ;
};

extern std::vector<AABB> colliders;

void addBox(float cx, float cz, float w, float d);
void initColliders();
bool checkCollision(float nx, float nz);

#endif
