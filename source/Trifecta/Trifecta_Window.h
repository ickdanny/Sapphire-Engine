#ifndef TRIFECTA_WINDOW_H
#define TRIFECTA_WINDOW_H

#include <stdbool.h>
#include <glfw3.h>

/* Represents a window */
typedef struct TFWindow{
    GLFWwindow* _windowPtr;
} TFWindow;

/* Constructs and returns a TFWindow by value */
TFWindow tfWindowMake(
    bool fullscreen,
    const char* windowName,
    int graphicsWidth,
    int graphicsHeight
);

/* Frees the given TFWindow */
void tfWindowFree(TFWindow *windowPtr);

#endif