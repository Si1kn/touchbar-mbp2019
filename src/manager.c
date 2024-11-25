
#include "../include/g_graphics.h"
#include "../include/manager.h"
#include <stdint.h>
#include <stdio.h>
static int test = 0;

uint32_t *uniPointer;

int width, height;

void startStep2(uint32_t *pointer, int w, int h) {
  uniPointer = pointer;
  width = w;
  height = h;

  printf("%d\n", width);
  printf("%d\n", height);
}
void loop() {

  int base = 2008 / 13;

  //	  g_renderBox(i*base -1 , i*base + 1, 0x808080);
  g_renderBox(0, base - 5, 0x808080);
  // updateDisplay();
  g_renderBox(base + 1, (base) * 2 + 1, 0x808080);
  updateDisplay();
}

void m_setPixel(int x, int y, int colour) {
  uniPointer[x * width + y] = colour;
}
