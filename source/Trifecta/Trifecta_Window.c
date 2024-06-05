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

/* Constructs and returns a TFWindow by value */
TFWindow tfWindowMake(
    bool fullscreen,
    const char* windowName,
    int graphicsWidth,
    int graphicsHeight
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
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
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

    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(toRet._windowPtr);

    return toRet;
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