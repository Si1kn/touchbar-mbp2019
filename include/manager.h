
#include <stdint.h>

static int running = 1;
void updateDisplay();
void loop();
void startStep2(uint32_t *pointer, int width, int height);
void m_setPixel(int x, int y, int colour);
