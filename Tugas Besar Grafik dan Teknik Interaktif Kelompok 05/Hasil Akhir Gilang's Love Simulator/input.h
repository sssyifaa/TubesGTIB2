#ifndef INPUT_H
#define INPUT_H

void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);

void specialKey(int key, int x, int y);
void specialKeyUp(int key, int x, int y);

void mouseMotion(int x, int y);

#endif
