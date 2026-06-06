#include "house.h"
#include "global.h"
#include "utils.h"
#include "texture.h"
#include <GL/glut.h>
#include <cstdlib>
#include <cmath>

static const float PI = 3.14159265f;

// ================================================================
//  Helper: Dinding Bata (dari teman)
// ================================================================

static void drawBrickWallX(float fx, float z0, float z1, float y0, float y1,
                            Color3 baseColor, float normalDir)
{
    float brickH  = 0.18f;
    float brickL  = 0.38f;
    float mortarV = 0.025f;
    float mortarH = 0.020f;
    Color3 mortarCol = makeColor(0.70f,0.68f,0.64f);
    Color3 brickDark = shade(baseColor, 0.82f);
    Color3 brickLight= shade(baseColor, 1.10f);
    float depth = 0.008f;

    glDisable(GL_LIGHTING);
    setColor(mortarCol);
    glNormal3f(normalDir, 0.f, 0.f);
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*depth, y0, z0);
        glVertex3f(fx+normalDir*depth, y0, z1);
        glVertex3f(fx+normalDir*depth, y1, z1);
        glVertex3f(fx+normalDir*depth, y1, z0);
    glEnd();
    glEnable(GL_LIGHTING);

    int nRows = (int)((y1-y0) / (brickH+mortarV)) + 1;
    int nCols = (int)((z1-z0) / (brickL+mortarH)) + 2;

    for(int row=0; row<nRows; row++){
        float yBot = y0 + row*(brickH+mortarV);
        float yTop = yBot + brickH;
        if(yTop > y1) yTop = y1;
        if(yBot >= y1) break;
        float offset = (row%2==0) ? 0.f : (brickL+mortarH)*0.5f;
        for(int col=-1; col<nCols; col++){
            float zLeft  = z0 + col*(brickL+mortarH) - offset + mortarH*0.5f;
            float zRight = zLeft + brickL;
            if(zRight <= z0) continue;
            if(zLeft  >= z1) break;
            float cl = (zLeft  < z0) ? z0 : zLeft;
            float cr = (zRight > z1) ? z1 : zRight;
            float var = 0.92f + 0.16f*(float)((row*13+col*7)%8)/7.f;
            Color3 bc = (row%3==0) ? shade(brickDark,var) : shade(brickLight,var);
            setColor(bc);
            float bDepth = depth+0.002f;
            glNormal3f(normalDir, 0.f, 0.f);
            glBegin(GL_QUADS);
                glVertex3f(fx+normalDir*bDepth, yBot, cl);
                glVertex3f(fx+normalDir*bDepth, yBot, cr);
                glVertex3f(fx+normalDir*bDepth, yTop, cr);
                glVertex3f(fx+normalDir*bDepth, yTop, cl);
            glEnd();
        }
    }
}

static void drawBrickWallZ(float fz, float x0, float x1, float y0, float y1,
                            Color3 baseColor, float normalDir)
{
    float brickH  = 0.18f;
    float brickL  = 0.38f;
    float mortarV = 0.025f;
    float mortarH = 0.020f;
    Color3 mortarCol = makeColor(0.70f,0.68f,0.64f);
    Color3 brickDark = shade(baseColor, 0.82f);
    Color3 brickLight= shade(baseColor, 1.10f);
    float depth = 0.008f;

    glDisable(GL_LIGHTING);
    setColor(mortarCol);
    glNormal3f(0.f, 0.f, normalDir);
    glBegin(GL_QUADS);
        glVertex3f(x0, y0, fz+normalDir*depth);
        glVertex3f(x1, y0, fz+normalDir*depth);
        glVertex3f(x1, y1, fz+normalDir*depth);
        glVertex3f(x0, y1, fz+normalDir*depth);
    glEnd();
    glEnable(GL_LIGHTING);

    int nRows = (int)((y1-y0) / (brickH+mortarV)) + 1;
    int nCols = (int)((x1-x0) / (brickL+mortarH)) + 2;

    for(int row=0; row<nRows; row++){
        float yBot = y0 + row*(brickH+mortarV);
        float yTop = yBot + brickH;
        if(yTop > y1) yTop = y1;
        if(yBot >= y1) break;
        float offset = (row%2==0) ? 0.f : (brickL+mortarH)*0.5f;
        for(int col=-1; col<nCols; col++){
            float xLeft  = x0 + col*(brickL+mortarH) - offset + mortarH*0.5f;
            float xRight = xLeft + brickL;
            if(xRight <= x0) continue;
            if(xLeft  >= x1) break;
            float cl = (xLeft  < x0) ? x0 : xLeft;
            float cr = (xRight > x1) ? x1 : xRight;
            float var = 0.92f + 0.16f*(float)((row*13+col*7)%8)/7.f;
            Color3 bc = (row%3==0) ? shade(brickDark,var) : shade(brickLight,var);
            setColor(bc);
            float bDepth = depth+0.002f;
            glNormal3f(0.f, 0.f, normalDir);
            glBegin(GL_QUADS);
                glVertex3f(cl, yBot, fz+normalDir*bDepth);
                glVertex3f(cr, yBot, fz+normalDir*bDepth);
                glVertex3f(cr, yTop, fz+normalDir*bDepth);
                glVertex3f(cl, yTop, fz+normalDir*bDepth);
            glEnd();
        }
    }
}

static void drawWindowX(float fx, float z0, float z1, float y0, float y1,
                         Color3 winC, float normalDir)
{
    float eps = 0.015f;
    setColor(winC);
    glNormal3f(normalDir,0,0);
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*eps, y0, z0);
        glVertex3f(fx+normalDir*eps, y0, z1);
        glVertex3f(fx+normalDir*eps, y1, z1);
        glVertex3f(fx+normalDir*eps, y1, z0);
    glEnd();
    Color3 fr = makeColor(0.50f,0.40f,0.25f);
    float fw=0.03f, e2=eps+0.005f;
    setColor(fr);
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*e2,y1,    z0-fw); glVertex3f(fx+normalDir*e2,y1,    z1+fw);
        glVertex3f(fx+normalDir*e2,y1+fw, z1+fw); glVertex3f(fx+normalDir*e2,y1+fw, z0-fw);
    glEnd();
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*e2,y0-fw, z0-fw); glVertex3f(fx+normalDir*e2,y0-fw, z1+fw);
        glVertex3f(fx+normalDir*e2,y0,    z1+fw); glVertex3f(fx+normalDir*e2,y0,    z0-fw);
    glEnd();
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*e2,y0-fw, z0-fw); glVertex3f(fx+normalDir*e2,y0-fw, z0);
        glVertex3f(fx+normalDir*e2,y1+fw, z0);    glVertex3f(fx+normalDir*e2,y1+fw, z0-fw);
    glEnd();
    glBegin(GL_QUADS);
        glVertex3f(fx+normalDir*e2,y0-fw, z1);    glVertex3f(fx+normalDir*e2,y0-fw, z1+fw);
        glVertex3f(fx+normalDir*e2,y1+fw, z1+fw); glVertex3f(fx+normalDir*e2,y1+fw, z1);
    glEnd();
}

// ================================================================
//  initHouses
// ================================================================
void initHouses() {
    srand(42);
    houses.clear();

    float roadHalf = 3.0f, startZ = 4.0f, spacing = 11.0f;
    int count = 10;

    static Color3 wallPal[] = {
        {0.86f,0.77f,0.62f},{0.91f,0.84f,0.72f},{0.79f,0.69f,0.56f},
        {0.93f,0.89f,0.81f},{0.72f,0.63f,0.52f},{0.82f,0.74f,0.64f}
    };
    static Color3 roofPal[] = {
        {0.48f,0.22f,0.12f},{0.55f,0.28f,0.14f},{0.36f,0.18f,0.10f},
        {0.30f,0.30f,0.30f},{0.20f,0.25f,0.30f}
    };

    for(int i=0; i<count; i++){
        for(int s=-1; s<=1; s+=2){
            House h;
            h.side = s;
            h.type = rand() % 3;
            if(h.type==0){ h.width=rnd(5.0f,6.0f); h.depth=rnd(5.5f,7.0f); h.wallH=rnd(2.8f,3.2f); h.roofH=rnd(1.8f,2.2f); }
            else if(h.type==1){ h.width=rnd(4.5f,5.5f); h.depth=rnd(5.0f,6.0f); h.wallH=rnd(3.2f,3.8f); h.roofH=rnd(2.5f,3.5f); }
            else { h.width=rnd(7.0f,8.5f); h.depth=rnd(5.0f,6.5f); h.wallH=rnd(2.5f,2.9f); h.roofH=rnd(1.5f,2.0f); }
            h.yardD = rnd(1.5f,3.0f);
            h.cz    = startZ - i*spacing + rnd(-0.8f,0.8f);
            float dist = roadHalf + h.yardD + h.depth*0.5f + rnd(0,0.5f);
            h.cx   = s * dist;
            h.wall = wallPal[rand()%6];
            h.roof = roofPal[rand()%5];
            h.door = makeColor(0.32f,0.18f,0.08f);
            h.win  = makeColor(0.60f,0.80f,0.92f);
            houses.push_back(h);
        }
    }

    // Rumah Bro
    {
        House h;
        h.side=1; h.type=1;
        h.width=5.0f; h.depth=6.0f; h.wallH=3.5f; h.roofH=3.0f; h.yardD=2.0f;
        h.cz=-106.0f; h.cx=8.0f;
        h.wall=wallPal[1]; h.roof=roofPal[2];
        h.door=makeColor(0.32f,0.18f,0.08f);
        h.win =makeColor(0.60f,0.80f,0.92f);
        houses.push_back(h);
    }
}

// ================================================================
//  drawHouse — versi lengkap dengan tekstur bata + jendela bingkai
// ================================================================
void drawHouse(const House &h) {
    float x0=h.cx-h.width*0.5f, x1=h.cx+h.width*0.5f;
    float z0=h.cz-h.depth*0.5f, z1=h.cz+h.depth*0.5f;
    float y0=0.f, y1=h.wallH;
    float ov=0.35f;

    Color3 wF=h.wall, wS=shade(h.wall,0.80f), wB=shade(h.wall,0.65f);

    // Dinding dasar (solid, supaya tidak ada lubang di sudut)
    setColor(wF); quad4(x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y1,z1);
    setColor(wB); quad4(x1,y0,z0, x0,y0,z0, x0,y1,z0, x1,y1,z0);
    setColor(wS); quad4(x0,y0,z0, x0,y0,z1, x0,y1,z1, x0,y1,z0);
    setColor(shade(h.wall,0.87f)); quad4(x1,y0,z1, x1,y0,z0, x1,y1,z0, x1,y1,z1);

    // Lapisan bata di atas dinding
    drawBrickWallZ(z1, x0, x1, y0, y1, wF, +1.f);
    drawBrickWallZ(z0, x0, x1, y0, y1, wB, -1.f);
    drawBrickWallX(x0, z0, z1, y0, y1, wS, -1.f);
    drawBrickWallX(x1, z0, z1, y0, y1, shade(h.wall,0.87f), +1.f);

    // Dimensi pintu & jendela
    float dw = h.width * 0.16f;
    float dh = h.wallH * 0.52f;
    float ww = h.width * 0.15f;
    float wh = h.wallH * 0.27f;
    float wb = y0 + h.wallH * 0.50f;
    float wt = wb + wh;

    if(h.side == -1){
        // Pintu di sisi x1 (muka menghadap kanan)
        float fx=x1, nDir=+1.f, eps=0.02f;
        float dzMid=(z0+z1)*0.5f;
        setColor(h.door);
        glNormal3f(nDir,0,0);
        glBegin(GL_QUADS);
            glVertex3f(fx+nDir*eps,y0, dzMid-dw*0.5f);
            glVertex3f(fx+nDir*eps,y0, dzMid+dw*0.5f);
            glVertex3f(fx+nDir*eps,dh, dzMid+dw*0.5f);
            glVertex3f(fx+nDir*eps,dh, dzMid-dw*0.5f);
        glEnd();
        // Bingkai pintu
        setColor(makeColor(0.50f,0.40f,0.25f));
        float pf=0.04f;
        glBegin(GL_QUADS);
            glVertex3f(fx+nDir*(eps+0.005f),dh,    dzMid-dw*0.5f-pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh,    dzMid+dw*0.5f+pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh+pf, dzMid+dw*0.5f+pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh+pf, dzMid-dw*0.5f-pf);
        glEnd();
        // 2 jendela
        float pz[2]={dzMid-h.depth*0.26f, dzMid+h.depth*0.26f};
        for(int i=0;i<2;i++) drawWindowX(fx, pz[i]-ww*0.5f, pz[i]+ww*0.5f, wb, wt, h.win, nDir);
    } else {
        // Pintu di sisi x0
        float fx=x0, nDir=-1.f, eps=0.02f;
        float dzMid=(z0+z1)*0.5f;
        setColor(h.door);
        glNormal3f(nDir,0,0);
        glBegin(GL_QUADS);
            glVertex3f(fx+nDir*eps,y0, dzMid-dw*0.5f);
            glVertex3f(fx+nDir*eps,y0, dzMid+dw*0.5f);
            glVertex3f(fx+nDir*eps,dh, dzMid+dw*0.5f);
            glVertex3f(fx+nDir*eps,dh, dzMid-dw*0.5f);
        glEnd();
        setColor(makeColor(0.50f,0.40f,0.25f));
        float pf=0.04f;
        glBegin(GL_QUADS);
            glVertex3f(fx+nDir*(eps+0.005f),dh,    dzMid-dw*0.5f-pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh,    dzMid+dw*0.5f+pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh+pf, dzMid+dw*0.5f+pf);
            glVertex3f(fx+nDir*(eps+0.005f),dh+pf, dzMid-dw*0.5f-pf);
        glEnd();
        float pz[2]={dzMid-h.depth*0.26f, dzMid+h.depth*0.26f};
        for(int i=0;i<2;i++) drawWindowX(fx, pz[i]-ww*0.5f, pz[i]+ww*0.5f, wb, wt, h.win, nDir);
    }

    // Atap pelana
    float rx0=x0-ov, rx1=x1+ov, rz0=z0-ov, rz1=z1+ov, rY=y1+h.roofH, rX=h.cx;
    setColor(h.roof);              quad4(rx0,y1,rz1, rx0,y1,rz0, rX,rY,rz0, rX,rY,rz1);
    setColor(shade(h.roof,0.68f)); quad4(rx1,y1,rz0, rx1,y1,rz1, rX,rY,rz1, rX,rY,rz0);
    setColor(shade(h.wall,0.92f)); tri3(rx0,y1,rz1, rx1,y1,rz1, rX,rY,rz1);
    setColor(shade(h.wall,0.72f)); tri3(rx1,y1,rz0, rx0,y1,rz0, rX,rY,rz0);
}

void drawHouses() {
    for(int i=0;i<(int)houses.size();i++) drawHouse(houses[i]);
}

// ================================================================
//  drawRumahInterior — ruangan rumah Bro yang lebih lengkap
// ================================================================
void drawRumahInterior() {
    float RW=20.0f, RD=20.0f, RH=7.0f;
    float cx=RUMAH_INTERIOR_X, cz=RUMAH_INTERIOR_Z;

    // --- Lantai kayu ---
    enableTex(texWood);
    setColor(0.70f,0.55f,0.35f);
    glNormal3f(0,1,0);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);   glVertex3f(cx-RW/2,0,cz+RD/2);
        glTexCoord2f(5,0);   glVertex3f(cx+RW/2,0,cz+RD/2);
        glTexCoord2f(5,5);   glVertex3f(cx+RW/2,0,cz-RD/2);
        glTexCoord2f(0,5);   glVertex3f(cx-RW/2,0,cz-RD/2);
    glEnd();
    disableTex();

    // --- Dinding ---
    setColor(0.88f,0.82f,0.72f);
    // Dinding belakang (z-)
    quad4(cx-RW/2,0,cz-RD/2, cx+RW/2,0,cz-RD/2, cx+RW/2,RH,cz-RD/2, cx-RW/2,RH,cz-RD/2);
    // Dinding kiri (x-)
    quad4(cx-RW/2,0,cz+RD/2, cx-RW/2,0,cz-RD/2, cx-RW/2,RH,cz-RD/2, cx-RW/2,RH,cz+RD/2);
    // Dinding kanan (x+)
    quad4(cx+RW/2,0,cz-RD/2, cx+RW/2,0,cz+RD/2, cx+RW/2,RH,cz+RD/2, cx+RW/2,RH,cz-RD/2);
    // Dinding depan (z+) — ada pintu di tengah
    setColor(0.82f,0.76f,0.68f);
    float doorW=2.0f, doorH=3.5f;
    // Kiri pintu
    quad4(cx-RW/2,0,cz+RD/2, cx-doorW/2,0,cz+RD/2, cx-doorW/2,RH,cz+RD/2, cx-RW/2,RH,cz+RD/2);
    // Kanan pintu
    quad4(cx+doorW/2,0,cz+RD/2, cx+RW/2,0,cz+RD/2, cx+RW/2,RH,cz+RD/2, cx+doorW/2,RH,cz+RD/2);
    // Atas pintu
    quad4(cx-doorW/2,doorH,cz+RD/2, cx+doorW/2,doorH,cz+RD/2, cx+doorW/2,RH,cz+RD/2, cx-doorW/2,RH,cz+RD/2);
    // Daun pintu
    setColor(0.38f,0.22f,0.10f);
    quad4(cx-doorW/2+0.05f,0,cz+RD/2+0.02f, cx+doorW/2-0.05f,0,cz+RD/2+0.02f,
          cx+doorW/2-0.05f,doorH,cz+RD/2+0.02f, cx-doorW/2+0.05f,doorH,cz+RD/2+0.02f);

    // --- Plafon ---
    setColor(0.95f,0.92f,0.88f);
    quad4(cx-RW/2,RH,cz-RD/2, cx+RW/2,RH,cz-RD/2,
          cx+RW/2,RH,cz+RD/2, cx-RW/2,RH,cz+RD/2);

    // --- Lampu gantung ---
    setColor(0.85f,0.72f,0.20f);
    glPushMatrix();
    glTranslatef(cx, RH-0.1f, cz);
    glScalef(0.5f,0.1f,0.5f);
    glutSolidCube(1.0f);
    glPopMatrix();
    setColor(0.95f,0.90f,0.50f);
    glPushMatrix();
    glTranslatef(cx, RH-0.5f, cz);
    glutSolidSphere(0.2f,12,8);
    glPopMatrix();

    // --- Meja di sudut kiri-belakang ---
    enableTex(texWood);
    setColor(0.55f,0.38f,0.20f);
    // Permukaan meja
    quad4(cx-RW/2,1.2f,cz-RD/2+3.0f, cx-RW/2+4.0f,1.2f,cz-RD/2+3.0f,
          cx-RW/2+4.0f,1.2f,cz-RD/2, cx-RW/2,1.2f,cz-RD/2);
    disableTex();
    // Kaki meja
    setColor(0.45f,0.30f,0.15f);
    float mx=cx-RW/2, mz=cz-RD/2;
    float legs[4][2]={{mx+0.2f,mz+0.2f},{mx+3.8f,mz+0.2f},{mx+0.2f,mz+2.8f},{mx+3.8f,mz+2.8f}};
    for(int i=0;i<4;i++){
        glPushMatrix();
        glTranslatef(legs[i][0],0.6f,legs[i][1]);
        glScalef(0.15f,1.2f,0.15f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // --- Kursi di depan meja ---
    setColor(0.50f,0.33f,0.18f);
    float chairX=cx-RW/2+2.0f, chairZ=cz-RD/2+3.8f;
    // Dudukan
    glPushMatrix(); glTranslatef(chairX,0.5f,chairZ); glScalef(0.8f,0.08f,0.8f); glutSolidCube(1.0f); glPopMatrix();
    // Sandaran
    glPushMatrix(); glTranslatef(chairX,0.9f,chairZ-0.36f); glScalef(0.8f,0.8f,0.08f); glutSolidCube(1.0f); glPopMatrix();
    // Kaki kursi
    float ck[4][2]={{-0.35f,-0.35f},{0.35f,-0.35f},{-0.35f,0.35f},{0.35f,0.35f}};
    for(int i=0;i<4;i++){
        glPushMatrix();
        glTranslatef(chairX+ck[i][0],0.25f,chairZ+ck[i][1]);
        glScalef(0.08f,0.5f,0.08f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // --- Rak buku di dinding kanan ---
    setColor(0.48f,0.30f,0.14f);
    float rsx=cx+RW/2-0.5f, rsz=cz-1.0f;
    // Badan rak
    glPushMatrix(); glTranslatef(rsx,2.0f,rsz); glScalef(0.3f,4.0f,3.0f); glutSolidCube(1.0f); glPopMatrix();
    // Buku-buku
    float bookColors[5][3]={{0.7f,0.2f,0.2f},{0.2f,0.5f,0.7f},{0.2f,0.7f,0.3f},{0.7f,0.6f,0.2f},{0.5f,0.2f,0.7f}};
    for(int i=0;i<5;i++){
        glColor3f(bookColors[i][0],bookColors[i][1],bookColors[i][2]);
        glPushMatrix();
        glTranslatef(rsx-0.1f, 0.5f+i*0.7f, rsz-1.0f+i*0.4f);
        glScalef(0.15f,0.6f,0.3f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // --- Ember (kalau belum diambil) ---
    if(!emberSudahDiambil){
        float ex=cx-RW/2+3.5f;
        float ez=cz-RD/2+3.5f;
        setColor(0.60f,0.60f,0.65f);
        glPushMatrix();
        glTranslatef(ex,1.25f,ez);
        GLUquadric* q=gluNewQuadric();
        glRotatef(-90,1,0,0);
        gluCylinder(q,0.20f,0.25f,0.35f,12,4);
        gluDisk(q,0,0.20f,12,1);
        // Handle ember
        glColor3f(0.70f,0.70f,0.75f);
        glTranslatef(0,0,0.35f);
        glPushMatrix();
        glTranslatef(0.22f,0,0);
        glRotatef(90,1,0,0);
        gluCylinder(q,0.03f,0.03f,0.5f,8,1);
        glPopMatrix();
        gluDeleteQuadric(q);
        glPopMatrix();

        // Label "EMBER" di atasnya
        setColor(1.0f,0.8f,0.2f);
        glRasterPos3f(ex-0.3f,1.8f,ez);
        const char* lbl="EMBER";
        for(const char* c=lbl;*c;c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c);
    }

    // --- Karpet di tengah ---
    setColor(0.65f,0.20f,0.20f);
    quad4(cx-3.0f,0.002f,cz+2.5f, cx+3.0f,0.002f,cz+2.5f,
          cx+3.0f,0.002f,cz-2.5f, cx-3.0f,0.002f,cz-2.5f);
    setColor(0.80f,0.40f,0.40f);
    quad4(cx-2.5f,0.003f,cz+2.0f, cx+2.5f,0.003f,cz+2.0f,
          cx+2.5f,0.003f,cz-2.0f, cx-2.5f,0.003f,cz-2.0f);
    setColor(0.65f,0.20f,0.20f);
    // Motif sederhana di tengah karpet
    quad4(cx-0.5f,0.004f,cz+0.5f, cx+0.5f,0.004f,cz+0.5f,
          cx+0.5f,0.004f,cz-0.5f, cx-0.5f,0.004f,cz-0.5f);
}

void drawHouseInside() { drawRumahInterior(); }

// ================================================================
//  Pagar, Gerbang, Kandang
// ================================================================
void drawFence(float x0,float z0,float x1,float z1,int posts){
    float dx=(x1-x0)/posts, dz=(z1-z0)/posts;
    Color3 wood=makeColor(0.50f,0.34f,0.18f);
    float railH[2]={0.72f,0.36f};
    for(int r=0;r<2;r++){
        float th=0.035f;
        setColor(shade(wood,r==0?1.0f:0.85f));
        glBegin(GL_QUADS);
            glVertex3f(x0,railH[r]-th,z0); glVertex3f(x1,railH[r]-th,z1);
            glVertex3f(x1,railH[r]+th,z1); glVertex3f(x0,railH[r]+th,z0);
        glEnd();
    }
    for(int i=0;i<=posts;i++){
        float px=x0+dx*i, pz=z0+dz*i;
        float lit=0.78f+0.22f*(float)(i%2);
        setColor(shade(wood,lit));
        float tw=0.045f;
        glBegin(GL_QUADS);
            glVertex3f(px-tw,0,pz); glVertex3f(px+tw,0,pz);
            glVertex3f(px+tw,0.90f,pz); glVertex3f(px-tw,0.90f,pz);
        glEnd();
        setColor(shade(wood,lit*0.90f));
        glBegin(GL_TRIANGLES);
            glVertex3f(px-tw,0.90f,pz); glVertex3f(px+tw,0.90f,pz);
            glVertex3f(px,1.06f,pz);
        glEnd();
    }
}

void drawGate(float hinge_x,float hinge_z,float gateAngleDeg,float gateWidth,bool isOpenLeft){
    Color3 wood=makeColor(0.45f,0.28f,0.12f);
    glPushMatrix();
    glTranslatef(hinge_x,0.0f,hinge_z);
    if(isOpenLeft) glRotatef(-gateAngleDeg,0,1,0);
    else           glRotatef( gateAngleDeg,0,1,0);
    setColor(wood);
    glPushMatrix(); glTranslatef(gateWidth*0.5f,0.36f,0); glScalef(gateWidth,0.08f,0.08f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(gateWidth*0.5f,0.72f,0); glScalef(gateWidth,0.08f,0.08f); glutSolidCube(1.0f); glPopMatrix();
    int boards=4;
    for(int i=0;i<boards;i++){
        float bx=gateWidth*(i+0.5f)/boards;
        glPushMatrix(); glTranslatef(bx,0.72f,0); glScalef(0.10f,1.44f,0.08f); glutSolidCube(1.0f); glPopMatrix();
    }
    glPopMatrix();
}

void drawKandangSapi(float cx,float cz){
    float w=8.0f,d=8.0f;
    float x0=cx-w*0.5f,x1=cx+w*0.5f,z0=cz-d*0.5f,z1=cz+d*0.5f;
    float gateW=2.0f;
    drawFence(x0,z0,x1,z0,8);
    drawFence(x0,z0,x0,z1,8);
    drawFence(x1,z0,x1,z1,8);
    drawFence(x0,z1,cx-gateW*0.5f,z1,3);
    drawFence(cx+gateW*0.5f,z1,x1,z1,3);
    drawGate(cx-gateW*0.5f,z1,gateSapiAngle,gateW,false);
    setColor(0.38f,0.28f,0.18f);
    glBegin(GL_QUADS);
        glVertex3f(x0,0.001f,z0); glVertex3f(x1,0.001f,z0);
        glVertex3f(x1,0.001f,z1); glVertex3f(x0,0.001f,z1);
    glEnd();
}

void drawKandangAyam(float cx,float cz){
    float w=8.0f,d=8.0f;
    float x0=cx-w*0.5f,x1=cx+w*0.5f,z0=cz-d*0.5f,z1=cz+d*0.5f;
    float gateW=2.0f;
    drawFence(x0,z0,x1,z0,8);
    drawFence(x0,z0,x0,z1,8);
    drawFence(x1,z0,x1,z1,8);
    drawFence(x0,z1,cx-gateW*0.5f,z1,3);
    drawFence(cx+gateW*0.5f,z1,x1,z1,3);
    drawGate(cx-gateW*0.5f,z1,gateAyamAngle,gateW,false);
    setColor(0.55f,0.42f,0.22f);
    glBegin(GL_QUADS);
        glVertex3f(x0,0.001f,z0); glVertex3f(x1,0.001f,z0);
        glVertex3f(x1,0.001f,z1); glVertex3f(x0,0.001f,z1);
    glEnd();
}
