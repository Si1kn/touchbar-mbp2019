typedef struct {
    int x, visable;
} gui_button;

typedef struct {
    gui_button *buttons;
} Menu;

void g_render(int *pixels);
