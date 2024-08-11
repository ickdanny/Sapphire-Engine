#ifndef TRIFECTA_WINDOW_H
#define TRIFECTA_WINDOW_H

#include <stdbool.h>

#ifdef __APPLE__

/* define to silence OpenGL deprecation warnings */
#define GL_SILENCE_DEPRECATION

/* define to have glfw3 include gl3 not gl */
#define GLFW_INCLUDE_GLCOREARB

#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h>

#endif /* end __APPLE__ */

#ifdef WIN32

#include "Trifecta_Win32.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#endif /* end WIN32 */

#ifdef __linux__

#include <GLFW/glfw3.h>
#include <GL/gl.h>

#endif /* end __linux__ */

#include "Constructure.h"

#include "Trifecta_Sprite.h"
#include "Trifecta_GlyphMap.h"
#include "_Trifecta_Graphics.h"

/* 
 * Represents a window which is capable of being
 * drawn on
 */
typedef struct TFWindow{
    GLFWwindow* _windowPtr;
    _TFGraphics _graphics;
    void *userPtr;
    void (*exitCallback)(void*);
    bool _fullscreen;
    int _windowWidth;
    int _windowHeight;
    int _windowX;
    int _windowY;
} TFWindow;

/* Constructs and returns a TFWindow by value */
TFWindow tfWindowMake(
    bool fullscreen,
    const char* windowName,
    int windowWidth,
    int windowHeight,
    int graphicsWidth,
    int graphicsHeight,
    void *userPtr
);

/* Makes the given TFWindow visible */
void tfWindowMakeVisible(TFWindow *windowPtr);

/*
 * Makes the given TFWindow fullscreen if it is
 * currently windowed, or windowed if it is currently
 * fullscreen; returns true if the window is now
 * fullscreen, false otherwise
 */
bool tfWindowToggleFullscreen(TFWindow *windowPtr);

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

/* 
 * Has the specified TFWindow deal with queued events
 * or messages
 */
void tfWindowPumpMessages(TFWindow *windowPtr);

/* Clears the depth buffer of the specified TFWindow */
void tfWindowClearDepth(TFWindow *windowPtr);

/* Draws a sprite onto the specified TFWindow */
void tfWindowDrawSprite(
    TFWindow *windowPtr,
    Point2D preOffsetCenter,
    const TFSpriteInstruction *spriteInstrPtr
);

/*
 * Draws a portion of a sprite onto the 
 * specified TFWindow
 */
void tfWindowDrawSubSprite(
    TFWindow *windowPtr,
    Point2D preOffsetCenter,
    const TFSpriteInstruction *spriteInstrPtr,
    const Rectangle *srcRectPtr
);

/* Draws a tiled sprite onto the specified TFWindow */
void tfWindowDrawTileSprite(
    TFWindow *windowPtr,
    const Rectangle *drawRectPtr,
    const TFSpriteInstruction *spriteInstrPtr,
    Point2D pixelOffset
);

/* Draws text onto the specified TFWindow */
void tfWindowDrawText(
    TFWindow *windowPtr,
    Point2D pos,
    const WideString *textPtr,
    int rightBound,
    TFGlyphMap *glyphMapPtr
);

/* Frees the given TFWindow */
void tfWindowFree(TFWindow *windowPtr);

#endif