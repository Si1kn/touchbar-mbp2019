#include "../include/g_graphics.h"
#include "../include/manager.h"

void g_renderBox(int startX, int endX, int colour) {
  for (int y = 0; y < 60; y++)
    for (int x = startX; x < endX; x++)
      m_setPixel(x, y, colour);
}
