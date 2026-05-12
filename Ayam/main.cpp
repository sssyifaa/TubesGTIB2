#ifndef IMAGE_LOADER_H_INCLUDED
#define IMAGE_LOADER_H_INCLUDED

class Image {
public:
    Image(char* ps, int w, int h);
    ~Image();
    char* pixels;
    int width;
    int height;
};

Image* loadBMP(const char* filename);

#endif

#include <assert.h>
#include <fstream>
#include "imageloader.h"

using namespace std;

Image::Image(char* ps, int w, int h) : pixels(ps), width(w), height(h) {}

Image::~Image() {
    delete[] pixels;
}

namespace {
    int toInt(const char* bytes) {
        return (int)(((unsigned char)bytes[3] << 24) |
                     ((unsigned char)bytes[2] << 16) |
                     ((unsigned char)bytes[1] << 8) |
                     (unsigned char)bytes[0]);
    }

    short toShort(const char* bytes) {
        return (short)(((unsigned char)bytes[1] << 8) |
                       (unsigned char)bytes[0]);
    }

    int readInt(ifstream &input) {
        char buffer[4];
        input.read(buffer, 4);
        return toInt(buffer);
    }

    short readShort(ifstream &input) {
        char buffer[2];
        input.read(buffer, 2);
        return toShort(buffer);
    }

    template<class T>
    class auto_array {
    private:
        T* array;
        mutable bool isReleased;
    public:
        explicit auto_array(T* array = NULL) : array(array), isReleased(false) {}
        auto_array(const auto_array<T> &aarray) {
            array = aarray.array;
            isReleased = aarray.isReleased;
            aarray.isReleased = true;
        }
        ~auto_array() {
            if (!isReleased && array != NULL) delete[] array;
        }
        T* get() const { return array; }
        T &operator*() const { return *array; }
        void operator=(const auto_array<T> &aarray) {
            if (!isReleased && array != NULL) delete[] array;
            array = aarray.array;
            isReleased = aarray.isReleased;
            aarray.isReleased = true;
        }
        T* operator->() const { return array; }
        T* release() {
            isReleased = true;
            return array;
        }
        T &operator[](int i) { return array[i]; }
    };
}

Image* loadBMP(const char* filename) {
    ifstream input;
    input.open(filename, ifstream::binary);
    assert(!input.fail() || !"File tidak ditemukan!!!");
    char buffer[2];
    input.read(buffer, 2);
    assert((buffer[0] == 'B' && buffer[1] == 'M') || !"Bukan file bitmap!!!");
    input.ignore(8);
    int dataOffset = readInt(input);
    int headerSize = readInt(input);
    int width, height;
    switch (headerSize) {
        case 40:
            width = readInt(input);
            height = readInt(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Gambar tidak 24 bits per pixel!");
            assert(readShort(input) == 0 || !"Gambar dikompres!");
            break;
        default:
            assert(!"Format bitmap tidak diketahui!");
    }
    int bytesPerRow = ((width * 3 + 3) / 4) * 4;
    int size = bytesPerRow * height;
    auto_array<char> pixels(new char[size]);
    input.seekg(dataOffset, ios_base::beg);
    input.read(pixels.get(), size);
    auto_array<char> pixels2(new char[width * height * 3]);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < 3; c++) {
                pixels2[3 * (width * y + x) + c] = pixels[bytesPerRow * y + 3 * x + (2 - c)];
            }
        }
    }
    input.close();
    return new Image(pixels2.release(), width, height);
}

#include <iostream>
#include <stdlib.h>
#include <GL/glut.h>
#include "imageloader.h"

using namespace std;

const float BOX_SIZE = 7.0f;
float _angle = 0;
GLuint _textureId;

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
        case 27: exit(0);
    }
}

GLuint loadTexture(Image* image) {
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
    return textureId;
}

void initRendering() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    Image* image = loadBMP("bg.bmp");
    _textureId = loadTexture(image);
    delete image;
}

void handleResize(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 1.0, 200.0);
}

void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -20.0f);
    glRotatef(-_angle, 1.0f, 1.0f, 0.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);

    // Depan
    glNormal3f(0.0, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-BOX_SIZE/2, -BOX_SIZE/2, BOX_SIZE/2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(BOX_SIZE/2, -BOX_SIZE/2, BOX_SIZE/2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(BOX_SIZE/2, BOX_SIZE/2, BOX_SIZE/2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-BOX_SIZE/2, BOX_SIZE/2, BOX_SIZE/2);

    // Belakang
    glNormal3f(0.0, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-BOX_SIZE/2, -BOX_SIZE/2, -BOX_SIZE/2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-BOX_SIZE/2, BOX_SIZE/2, -BOX_SIZE/2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(BOX_SIZE/2, BOX_SIZE/2, -BOX_SIZE/2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(BOX_SIZE/2, -BOX_SIZE/2, -BOX_SIZE/2);

    // Kiri
    glNormal3f(-1.0, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-BOX_SIZE/2, -BOX_SIZE/2, -BOX_SIZE/2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-BOX_SIZE/2, -BOX_SIZE/2, BOX_SIZE/2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-BOX_SIZE/2, BOX_SIZE/2, BOX_SIZE/2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-BOX_SIZE/2, BOX_SIZE/2, -BOX_SIZE/2);

    // Kanan
    glNormal3f(1.0, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(BOX_SIZE/2, -BOX_SIZE/2, -BOX_SIZE/2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(BOX_SIZE/2, -BOX_SIZE/2, BOX_SIZE/2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(BOX_SIZE/2, BOX_SIZE/2, BOX_SIZE/2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(BOX_SIZE/2, BOX_SIZE/2, -BOX_SIZE/2);

    // Atas
    glNormal3f(0.0, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-BOX_SIZE/2, BOX_SIZE/2, -BOX_SIZE/2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-BOX_SIZE/2, BOX_SIZE/2, BOX_SIZE/2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(BOX_SIZE/2, BOX_SIZE/2, BOX_SIZE/2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(BOX_SIZE/2, BOX_SIZE/2, -BOX_SIZE/2);

    // Bawah
    glNormal3f(0.0, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-BOX_SIZE/2, -BOX_SIZE/2, -BOX_SIZE/2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(BOX_SIZE/2, -BOX_SIZE/2, -BOX_SIZE/2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(BOX_SIZE/2, -BOX_SIZE/2, BOX_SIZE/2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-BOX_SIZE/2, -BOX_SIZE/2, BOX_SIZE/2);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glutSwapBuffers();
}

void update(int value) {
    _angle += 1.0f;
    if (_angle > 360) _angle -= 360;
    glutPostRedisplay();
    glutTimerFunc(25, update, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(400, 400);
    glutCreateWindow("Texture Mapping - Praktikum GTI B2");
    initRendering();
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(handleKeypress);
    glutReshapeFunc(handleResize);
    glutTimerFunc(25, update, 0);
    glutMainLoop();
    return 0;
}
