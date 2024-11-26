
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

void loadOnce() {
    int base = 2008 / 12 - 4;

    int spacing = base + 5; // Desired space between boxes

    for (int i = 0; i < 12; i++) {
        int c = i * spacing; // Increment `c` to include box width and spacing

        gui_button b1;
        b1.x = c;
        b1.visable = 1;
        b[i] = b1;

        // g_renderBox(c, c + base, 0x803380, tempPixels);
    }
}


void loop() {
 loadOnce();
  int base = 2008 / 13;

  //	  g_renderBox(i*base -1 , i*base + 1, 0x808080);
 // g_renderBox(0, base - 5, 0x808080);
  // updateDisplay();
 // g_renderBox(base + 1, (base) * 2 + 1, 0x808080);
 
   int base = 2008 / 12 - 4;

    int spacing = base + 5; // Desired space between boxes

     for (int i = 0; i < 12; i++) {
        gui_button b1 = b[i];

        g_renderBox(b1.x, b1.x + base, 0x803380, tempPixels);
    }
   updateDisplay();

}

void m_setPixel(int x, int y, int colour) {
  uniPointer[x * width + y] = colour;
}
