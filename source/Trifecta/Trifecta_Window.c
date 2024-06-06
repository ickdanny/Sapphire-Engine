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

/* Constructs and returns a TFWindow by value */
TFWindow tfWindowMake(
    bool fullscreen,
    const char* windowName,
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    //glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    toRet._windowPtr = glfwCreateWindow(
        graphicsWidth,
        graphicsHeight,
        windowName,
        fullscreen ? glfwGetPrimaryMonitor() : NULL,
        NULL
    );
    assertNotNull(
        toRet._windowPtr,
        "glfwCreateWindow() error"
    );

    glfwMakeContextCurrent(toRet._windowPtr);

    /* set the specified user ptr */
    toRet.userPtr = userPtr;

    return toRet;
}

/* Makes the given TFWindow visible */
void tfWindowMakeVisible(TFWindow *windowPtr){
    glfwShowWindow(windowPtr->_windowPtr);
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
    glClearColor(1.0, 0.5, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(windowPtr->_windowPtr);
    glfwPollEvents();
}

/* Frees the given TFWindow */
void tfWindowFree(TFWindow *windowPtr){
    if(windowPtr){
        glfwDestroyWindow(windowPtr->_windowPtr);
        windowPtr->_windowPtr = NULL;
        glfwTerminate();
        glfwInitialized = false;
    }
}