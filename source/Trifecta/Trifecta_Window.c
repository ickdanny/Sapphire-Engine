#include "Trifecta_Window.h"

#include "PGUtil.h"

static bool glfwInitialized = false;

/* GLFW error callback for debugging */
static void errorCallbackFunc(
    int errorCode,
    const char* description
){
    pgError(description);
}

/* GLFW window exit callback for a TFWindow */
static void exitCallbackFunc(
    GLFWwindow *glfwWindowPtr
){
    TFWindow *windowPtr
        = glfwGetWindowUserPointer(glfwWindowPtr);
    (windowPtr->exitCallback)(windowPtr->userPtr);
}

/* returns the width of the primary monitor */
static int getPrimaryMonitorWidth(){
    const GLFWvidmode *modePtr = glfwGetVideoMode(
        glfwGetPrimaryMonitor()
    );
    return modePtr->width;
}

/* returns the height of the primary monitor */
static int getPrimaryMonitorHeight(){
    const GLFWvidmode *modePtr = glfwGetVideoMode(
        glfwGetPrimaryMonitor()
    );
    return modePtr->height;
}

/* updates the stored window position of a TFWindow */
void tfWindowStorePos(TFWindow *windowPtr){
    glfwGetWindowPos(
        windowPtr->_windowPtr,
        &(windowPtr->_windowX),
        &(windowPtr->_windowY)
    );
}

/* Constructs and returns a TFWindow by value */
TFWindow tfWindowMake(
    bool fullscreen,
    const char* windowName,
    int windowWidth,
    int windowHeight,
    int graphicsWidth,
    int graphicsHeight,
    void *userPtr
){
    /* call glfwInit if needed */
    if(!glfwInitialized){
        assertTrue(
            glfwInit() == GL_TRUE,
            "glfwInit() error"
        );
        glfwSetErrorCallback(errorCallbackFunc);
        glfwInitialized = true;
    }

    TFWindow toRet = {0};

    /* make window */
    glfwWindowHint(
        GLFW_OPENGL_FORWARD_COMPAT,
        GL_TRUE
    );
    glfwWindowHint(GLFW_SAMPLES, 4);
    /* opengl version 3.3 */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    if(fullscreen){
        toRet._windowPtr = glfwCreateWindow(
            getPrimaryMonitorWidth(),
            getPrimaryMonitorHeight(),
            windowName,
            glfwGetPrimaryMonitor(),
            NULL
        );
    }
    else{
        toRet._windowPtr = glfwCreateWindow(
            windowWidth,
            windowHeight,
            windowName,
            NULL,
            NULL
        );
        tfWindowStorePos(&toRet);
    }
    assertNotNull(
        toRet._windowPtr,
        "glfwCreateWindow() error; "
        SRC_LOCATION
    );

    glfwMakeContextCurrent(toRet._windowPtr);

    /* initialize graphics */
    toRet._graphics = _tfGraphicsMake(
        graphicsWidth,
        graphicsHeight
    );

    /* set the specified user ptr */
    toRet.userPtr = userPtr;

    toRet._fullscreen = fullscreen;
    toRet._windowWidth = windowWidth;
    toRet._windowHeight = windowHeight;

    return toRet;
}

/* Makes the given TFWindow visible */
void tfWindowMakeVisible(TFWindow *windowPtr){
    glfwShowWindow(windowPtr->_windowPtr);
}

/*
 * Makes the given TFWindow fullscreen if it is
 * currently windowed, or windowed if it is currently
 * fullscreen; returns true if the window is now
 * fullscreen, false otherwise
 */
bool tfWindowToggleFullscreen(TFWindow *windowPtr){
    /* if window is fullscreen, make it windowed */
    if(windowPtr->_fullscreen){
        glfwSetWindowMonitor(
            windowPtr->_windowPtr,
            NULL,
            windowPtr->_windowX,
            windowPtr->_windowY,
            windowPtr->_windowWidth,
            windowPtr->_windowHeight,
            GLFW_DONT_CARE
        );
    }
    /* if window is windowed, make it fullscreen */
    else{
        tfWindowStorePos(windowPtr);
        glfwSetWindowMonitor(
            windowPtr->_windowPtr,
            glfwGetPrimaryMonitor(),
            0,
            0,
            getPrimaryMonitorWidth(),
            getPrimaryMonitorHeight(),
            GLFW_DONT_CARE
        );
    }
    windowPtr->_fullscreen = !windowPtr->_fullscreen;
    return windowPtr->_fullscreen;
}

/* 
 * Sets the exit callback for the given TFWindow;
 * the callback function takes a parameter of type
 * void* and will be passed the userPtr of the
 * given TFWindow
 */
void tfWindowSetExitCallback(
    TFWindow *windowPtr,
    void (*exitCallback)(void*)
){
    windowPtr->exitCallback = exitCallback;
    
    /* set the window user ptr to the TFWindow */
    glfwSetWindowUserPointer(
        windowPtr->_windowPtr,
        windowPtr
    );

    /* set the glfw window exit callback func */
    glfwSetWindowCloseCallback(
        windowPtr->_windowPtr,
        exitCallbackFunc
    );
}

/* Has the specified TFWindow render */
void tfWindowRender(TFWindow *windowPtr){
    //todo window rendering
    glfwSwapBuffers(windowPtr->_windowPtr);

    glClearColor(1.0, 0.5, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    tfWindowPumpMessages(windowPtr);
}

/* 
 * Has the specified TFWindow deal with queued events
 * or messages
 */
void tfWindowPumpMessages(TFWindow *windowPtr){
    glfwPollEvents();
}

/* Frees the given TFWindow */
void tfWindowFree(TFWindow *windowPtr){
    if(windowPtr){
        _tfGraphicsFree(&(windowPtr->_graphics));
        glfwDestroyWindow(windowPtr->_windowPtr);
        windowPtr->_windowPtr = NULL;
        glfwTerminate();
        glfwInitialized = false;
    }
}