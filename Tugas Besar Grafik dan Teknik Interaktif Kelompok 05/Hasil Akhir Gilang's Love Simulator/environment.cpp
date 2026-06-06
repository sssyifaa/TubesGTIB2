#include "environment.h"
#include "global.h"
#include "utils.h"
#include "texture.h"
#include "house.h"
#include "cow.h"
#include "chiken.h"
#include "shadow.h"
#include "player.h"
#include "tisya.h"
#include "bro.h"
#include "mission.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>

static const float PI = 3.14159265f;

// ================================================================
//  Helper pohon (dari mainApanih)
// ================================================================
static float cylX_(float r, float a) { return r * cosf(a); }
static float cylZ__(float r, float a) { return r * sinf(a); }

static void drawTreeNatural(float tx, float tz, unsigned int seed, bool hasApples=false) {
    srand(seed);
    float trunkH=rnd(2.5f,4.5f), rBot=0.13f+rnd(0,0.04f), rTop=0.05f+rnd(0,0.02f);
    Color3 bark=makeColor(0.30f+rnd(-0.05f,0.05f),0.20f+rnd(-0.03f,0.03f),0.10f+rnd(-0.02f,0.02f));
    int cseg=10;
    float hLevels[4]={0,trunkH*0.35f,trunkH*0.72f,trunkH};
    float rLevels[4]={rBot,rBot*0.82f,rTop*1.4f,rTop};
    for(int s=0;s<3;s++) {
        float ys0=hLevels[s],ys1=hLevels[s+1],rs0=rLevels[s],rs1=rLevels[s+1];
        for(int i=0;i<cseg;i++) {
            float a0=(float)i/cseg*2*PI, a1=(float)(i+1)/cseg*2*PI;
            float lit=0.70f+0.30f*(cosf((a0+a1)*0.5f)*0.5f+0.5f);
            setColor(shade(bark,lit));
            glBegin(GL_QUADS);
                glNormal3f(cosf(a0),0.15f,sinf(a0));
                glVertex3f(tx+cylX_(rs0,a0),ys0,tz+cylZ__(rs0,a0));
                glVertex3f(tx+cylX_(rs1,a0),ys1,tz+cylZ__(rs1,a0));
                glVertex3f(tx+cylX_(rs1,a1),ys1,tz+cylZ__(rs1,a1));
                glVertex3f(tx+cylX_(rs0,a1),ys0,tz+cylZ__(rs0,a1));
            glEnd();
        }
    }
    Color3 leafBase = hasApples
        ? makeColor(0.20f,0.55f,0.12f)
        : makeColor(0.16f+rnd(-0.04f,0.04f),0.40f+rnd(-0.06f,0.06f),0.12f+rnd(-0.03f,0.03f));
    float crownBase=trunkH*0.38f, crownR=rnd(1.2f,2.0f);
    int layers=5;
    for(int L=0;L<layers;L++) {
        float lf=(float)L/(layers-1), layerY=crownBase+lf*(trunkH*0.65f), offX=rnd(-0.2f,0.2f);
        float layerR=crownR*(1.0f-lf*0.55f)*rnd(0.85f,1.15f), layerH=crownR*0.60f*(1.0f-lf*0.28f);
        Color3 lColor=makeColor(clampF(leafBase.r+0.04f*L,0,1),clampF(leafBase.g+0.05f*L,0,1),clampF(leafBase.b+0.02f*L,0,1));
        int lseg=14;
        for(int i=0;i<lseg;i++) {
            float a0=(float)i/lseg*2*PI, a1=(float)(i+1)/lseg*2*PI, am=(a0+a1)*0.5f;
            float sunLit=0.72f+0.28f*(cosf(am)*0.55f+0.45f);
            setColor(shade(lColor,sunLit));
            glBegin(GL_TRIANGLES);
                glNormal3f(cosf(am),0.50f,sinf(am));
                glVertex3f(tx+offX+layerR*cosf(a0),layerY,tz+layerR*sinf(a0));
                glVertex3f(tx+offX+layerR*cosf(a1),layerY,tz+layerR*sinf(a1));
                glVertex3f(tx+offX,layerY+layerH,tz);
            glEnd();
            if(hasApples && (i%4==0)) {
                setColor(0.85f,0.10f,0.10f);
                glPushMatrix();
                glTranslatef(tx+offX+layerR*cosf(am)*0.80f, layerY+layerH*0.25f, tz+layerR*sinf(am)*0.80f);
                glutSolidSphere(0.13f,6,6);
                glPopMatrix();
            }
        }
        setColor(shade(lColor,0.55f));
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0,-1,0);
        glVertex3f(tx+offX,layerY,tz);
        for(int i=0;i<=lseg;i++){float a=(float)i/lseg*2*PI; glVertex3f(tx+offX+layerR*cosf(a),layerY,tz+layerR*sinf(a));}
        glEnd();
    }
}

// ================================================================
//  Tanah, Jalan, Langit
// ================================================================
void drawRoad() {
    float rw=6.0f, zN=20.0f, zF=-150.0f, sw=1.4f, ts=0.1f;
    enableTex(texRoad); glNormal3f(0,1,0);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);          glVertex3f(-rw*.5f,0.005f,zN);
        glTexCoord2f(1,0);          glVertex3f( rw*.5f,0.005f,zN);
        glTexCoord2f(1,(zN-zF)*ts); glVertex3f( rw*.5f,0.005f,zF);
        glTexCoord2f(0,(zN-zF)*ts); glVertex3f(-rw*.5f,0.005f,zF);
    glEnd(); disableTex();
    enableTex(texDirt); glNormal3f(0,1,0);
    for(int s=-1;s<=1;s+=2) {
        float sx0=s*rw*.5f, sx1=s*(rw*.5f+sw);
        glBegin(GL_QUADS);
            glTexCoord2f(0,0);          glVertex3f(sx0,0.003f,zN);
            glTexCoord2f(1,0);          glVertex3f(sx1,0.003f,zN);
            glTexCoord2f(1,(zN-zF)*ts); glVertex3f(sx1,0.003f,zF);
            glTexCoord2f(0,(zN-zF)*ts); glVertex3f(sx0,0.003f,zF);
        glEnd();
    }
    disableTex();
}

void drawGround() {
    enableTex(texDirt); glNormal3f(0,1,0);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);   glVertex3f(-300,0,25);
        glTexCoord2f(60,0);  glVertex3f( 300,0,25);
        glTexCoord2f(60,45); glVertex3f( 300,0,-200);
        glTexCoord2f(0,45);  glVertex3f(-300,0,-200);
    glEnd(); disableTex();
    enableTex(texGrass); glNormal3f(0,1,0);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);   glVertex3f(-100,0.002f,22);
        glTexCoord2f(20,0);  glVertex3f( 100,0.002f,22);
        glTexCoord2f(20,40); glVertex3f( 100,0.002f,-180);
        glTexCoord2f(0,40);  glVertex3f(-100,0.002f,-180);
    glEnd(); disableTex();
}

void drawSky() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    float zS=-180.0f, xW=300.0f;
    struct SkyBand{float y0,y1,r0,g0,b0,r1,g1,b1;};
    SkyBand bands[]={
        {0,  8,  0.72f,0.62f,0.55f, 0.65f,0.55f,0.60f},
        {8,  20, 0.65f,0.55f,0.60f, 0.45f,0.50f,0.70f},
        {20, 45, 0.45f,0.50f,0.70f, 0.25f,0.35f,0.65f},
    };
    for(int i=0;i<3;i++) {
        SkyBand &b=bands[i];
        glBegin(GL_QUADS);
            glColor3f(b.r0,b.g0,b.b0); glVertex3f(-xW,b.y0,zS); glVertex3f(xW,b.y0,zS);
            glColor3f(b.r1,b.g1,b.b1); glVertex3f(xW,b.y1,zS);  glVertex3f(-xW,b.y1,zS);
        glEnd();
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ================================================================
//  drawScene - sesuai mainApanih
// ================================================================
void drawScene() {
    drawSky();
    drawGround();
    drawRoad();

    for(int i = 0; i < (int)houses.size(); i++)
        drawHouse(houses[i]);

    // Kandang sapi + tong + sapi
    {
        float kx = 22.0f, kz = -35.0f;
        TONG_X = kx + 2.5f;
        TONG_Z = kz + 1.0f;
        drawKandangSapi(kx, kz);
        drawCow();
        drawTong();
    }

    // Kandang ayam + ayam
    {
        drawKandangAyam(CHICKEN_CX, CHICKEN_CZ);
        for(int i = 0; i < NUM_CHICKENS; i++)
            drawSingleChicken(chickens[i].x, chickens[i].z,
                              chickens[i].angle, chickens[i].legAnim);
    }

    // Pohon apel
    srand(55);
    for(int row = 0; row < 4; row++) {
        for(int col = 0; col < 5; col++) {
            float ax = -15.0f - col*5.0f + rnd(-0.5f,0.5f);
            float az = -20.0f - row*6.0f + rnd(-0.5f,0.5f);
            int idx = row*5 + col;
            if((int)treesApple.size() <= idx) {
                TreePos tp; tp.x=ax; tp.z=az;
                treesApple.push_back(tp);
            }
            drawTreeNatural(ax, az, (unsigned)(row*10+col), true);
        }
    }

    // Pagar halaman rumah
    for(int i = 0; i < (int)houses.size(); i += 2) {
        const House &h = houses[i];
        drawFence(h.cx - h.width*0.5f, h.cz + h.depth*0.5f + 0.08f,
                  h.cx + h.width*0.5f, h.cz + h.depth*0.5f + 0.08f, 8);
    }

    // Apel jatuh (Level 1)
    if(currentMission == MISSION_APEL && applesInitialized) {
        drawAppleItems();
    }

    // Telur (Level 2)
    if(currentMission == MISSION_TELUR && eggsInitialized) {
        drawEggItems();
    }

    // Shadow
    drawCowShadow();
    for(int i = 0; i < NUM_CHICKENS; i++)
        drawChickenShadow(chickens[i].x, chickens[i].z, chickens[i].angle);
    drawPlayerShadow();
    drawTisyaShadow();

    // Karakter
    drawPlayer();
    drawBro();
    drawTisya();
}

// ================================================================
//  Pencahayaan & Fog
// ================================================================
void setupEnvironment() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glDisable(GL_LIGHT2); // matikan lampu interior saat outdoor
    glDisable(GL_LIGHT3);

    GLfloat sunPos[]={-15,12,10,0};
    GLfloat sunAmb[]={0.25f,0.20f,0.15f,1};
    GLfloat sunDif[]={1.00f,0.85f,0.65f,1};
    GLfloat sunSpc[]={0.30f,0.25f,0.20f,1};
    glLightfv(GL_LIGHT0,GL_POSITION,sunPos);
    glLightfv(GL_LIGHT0,GL_AMBIENT, sunAmb);
    glLightfv(GL_LIGHT0,GL_DIFFUSE, sunDif);
    glLightfv(GL_LIGHT0,GL_SPECULAR,sunSpc);

    GLfloat skyPos[]={10,20,-10,0};
    GLfloat skyDif[]={0.20f,0.25f,0.35f,1};
    GLfloat skyAmb[]={0.10f,0.15f,0.20f,1};
    GLfloat noSpec[]={0,0,0,1};
    glLightfv(GL_LIGHT1,GL_POSITION,skyPos);
    glLightfv(GL_LIGHT1,GL_DIFFUSE, skyDif);
    glLightfv(GL_LIGHT1,GL_AMBIENT, skyAmb);
    glLightfv(GL_LIGHT1,GL_SPECULAR,noSpec);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);

    GLfloat matSpc[]={0.05f,0.05f,0.05f,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,matSpc);
    glMaterialf (GL_FRONT_AND_BACK,GL_SHININESS,5.0f);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE,GL_EXP2);
    GLfloat fogColor[]={0.72f,0.62f,0.55f,1.0f};
    glFogfv(GL_FOG_COLOR,fogColor);
    glFogf(GL_FOG_DENSITY,0.012f);
    glHint(GL_FOG_HINT,GL_NICEST);
}

void setupInteriorLighting() {
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_FOG);

    // Lampu utama di tengah atap
    glEnable(GL_LIGHT2);
    float cx=RUMAH_INTERIOR_X, cz=RUMAH_INTERIOR_Z;
    GLfloat lampPos[]={cx, 6.0f, cz, 1.0f};
    GLfloat lampAmb[]={0.55f, 0.45f, 0.30f, 1.0f};  // ambient lebih terang
    GLfloat lampDif[]={1.00f, 0.90f, 0.70f, 1.0f};
    GLfloat lampSpc[]={0.30f, 0.25f, 0.15f, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION,  lampPos);
    glLightfv(GL_LIGHT2, GL_AMBIENT,   lampAmb);
    glLightfv(GL_LIGHT2, GL_DIFFUSE,   lampDif);
    glLightfv(GL_LIGHT2, GL_SPECULAR,  lampSpc);
    glLightf (GL_LIGHT2, GL_CONSTANT_ATTENUATION,  0.3f);
    glLightf (GL_LIGHT2, GL_LINEAR_ATTENUATION,    0.01f);
    glLightf (GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.002f);

    // Lampu fill dari bawah supaya tidak ada sudut gelap total
    glEnable(GL_LIGHT3);
    GLfloat fillPos[]={cx, 0.5f, cz, 1.0f};
    GLfloat fillAmb[]={0.20f, 0.15f, 0.10f, 1.0f};
    GLfloat fillDif[]={0.30f, 0.25f, 0.20f, 1.0f};
    GLfloat noSpec[] ={0.0f,  0.0f,  0.0f,  1.0f};
    glLightfv(GL_LIGHT3, GL_POSITION, fillPos);
    glLightfv(GL_LIGHT3, GL_AMBIENT,  fillAmb);
    glLightfv(GL_LIGHT3, GL_DIFFUSE,  fillDif);
    glLightfv(GL_LIGHT3, GL_SPECULAR, noSpec);
    glLightf (GL_LIGHT3, GL_CONSTANT_ATTENUATION,  0.5f);
    glLightf (GL_LIGHT3, GL_LINEAR_ATTENUATION,    0.02f);
    glLightf (GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.005f);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}
