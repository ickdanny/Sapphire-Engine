#ifndef TRIFECTA_WINDOW_H
#define TRIFECTA_WINDOW_H

#include <stdbool.h>
#include <GLFW/glfw3.h>
#include <OpenGL/gl.h>

//todo: draw instruction in this file? outside Trifecta?

/* 
 * Represents a window which is capable of being
 * drawn to
 */
typedef struct TFWindow{
    GLFWwindow* _windowPtr;
    void *userPtr;
    void (*exitCallback)(void*);
} TFWindow;

/* Constructs and returns a TFWindow by value */
TFWindow tfWindowMake(
    bool fullscreen,
    const char* windowName,
    int graphicsWidth,
    int graphicsHeight,
    void *userPtr
);

/* Makes the given TFWindow visible */
void tfWindowMakeVisible(TFWindow *windowPtr);

/* 
 * Sets the exit callback for the given TFWindow;
 * the callback function takes a parameter of type
 * void* and will be passed the userPtr of the
 * given TFWindow
 */
void tfWindowSetExitCallback(
    TFWindow *windowPtr,
    void (*exitCallback)(void*)
);

/* Has the specified TFWindow render */
void tfWindowRender(TFWindow *windowPtr);

/* Frees the given TFWindow */
void tfWindowFree(TFWindow *windowPtr);

#endif