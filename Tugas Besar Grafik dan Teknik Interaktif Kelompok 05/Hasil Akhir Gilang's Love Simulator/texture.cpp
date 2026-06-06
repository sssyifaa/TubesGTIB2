#include "texture.h"
#include <cmath>
#include <cstdlib>

GLuint texGrass = 0;
GLuint texDirt  = 0;
GLuint texRoad  = 0;
GLuint texWood  = 0;

static float smoothNoise(int x, int y, int seed) {
    int n = x * 1619 + y * 31337 + seed * 3791;
    n = (n << 13) ^ n;
    return 1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
}

static GLuint makeTexture(unsigned char* data, int w, int h) {
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);
    return id;
}

static unsigned char clampByte(int v) {
    return (unsigned char)(v < 0 ? 0 : v > 255 ? 255 : v);
}

void generateTextures() {
    const int SZ = 128;
    unsigned char buf[SZ * SZ * 3];

    // RUMPUT
    for(int y=0;y<SZ;y++) for(int x=0;x<SZ;x++) {
        float n=(smoothNoise(x,y,1)*0.5f+smoothNoise(x/2,y/2,2)*0.3f+smoothNoise(x/4,y/4,3)*0.2f)*0.5f+0.5f;
        int idx=(y*SZ+x)*3;
        buf[idx+0]=clampByte((int)(58+n*18)+((x*7+y*13)%40==0?5:0));
        buf[idx+1]=clampByte((int)(130+n*25)+((x*7+y*13)%40==0?20:0));
        buf[idx+2]=clampByte((int)(45+n*12));
    }
    texGrass=makeTexture(buf,SZ,SZ);

    // TANAH
    for(int y=0;y<SZ;y++) for(int x=0;x<SZ;x++) {
        float n=(smoothNoise(x,y,10)*0.5f+smoothNoise(x/2,y/2,11)*0.3f+smoothNoise(x/4,y/4,12)*0.2f)*0.5f+0.5f;
        int idx=(y*SZ+x)*3;
        buf[idx+0]=clampByte((int)(120+n*30));
        buf[idx+1]=clampByte((int)(88+n*20));
        buf[idx+2]=clampByte((int)(55+n*12));
    }
    texDirt=makeTexture(buf,SZ,SZ);

    // ASPAL
    for(int y=0;y<SZ;y++) for(int x=0;x<SZ;x++) {
        float n=(smoothNoise(x,y,20)*0.4f+smoothNoise(x/2,y/2,21)*0.3f)*0.5f+0.5f;
        int v=(int)(55+n*20);
        if(x>=60&&x<=68&&(y/16)%2==0) v=210;
        int idx=(y*SZ+x)*3;
        buf[idx+0]=buf[idx+1]=buf[idx+2]=clampByte(v);
    }
    texRoad=makeTexture(buf,SZ,SZ);

    // KAYU
    for(int y=0;y<SZ;y++) for(int x=0;x<SZ;x++) {
        int plank=y/16;
        float plankPos=(float)(y%16)/16.0f;
        float grain=(smoothNoise(x/3,plank,30)*0.4f+smoothNoise(x/6,plank,31)*0.3f)*0.5f+0.5f;
        float edge=0.75f+(1.0f-2.0f*fabsf(plankPos-0.5f))*0.25f;
        float base=(plank%2==0)?0.85f:0.78f;
        int r=(int)(160*base*edge*grain),g=(int)(105*base*edge*grain),b=(int)(55*base*edge*grain);
        if(y%16==0||y%16==15){r/=2;g/=2;b/=2;}
        int idx=(y*SZ+x)*3;
        buf[idx+0]=clampByte(r);buf[idx+1]=clampByte(g);buf[idx+2]=clampByte(b);
    }
    texWood=makeTexture(buf,SZ,SZ);
}

void enableTex(GLuint id) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void disableTex() { glDisable(GL_TEXTURE_2D); }
