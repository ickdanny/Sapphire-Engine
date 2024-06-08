#include "_Trifecta_Graphics.h"

#include "PGUtil.h"

#include "_Trifecta_Shaders.h"

/*
#ifdef __APPLE__

#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE

#endif
*/

/* Debug function to check for OpenGL errors */
static void checkGLError(){
    switch(glGetError()){
        case GL_NO_ERROR:
            /* do nothing */
            break;
        case GL_INVALID_ENUM:
            pgWarning("OpenGL invalid enum");
            break;
        case GL_INVALID_VALUE:
            pgWarning("OpenGL invalid value");
            break;
        case GL_INVALID_OPERATION:
            pgWarning("OpenGL invalid operation");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            pgWarning(
                "OpenGL invalid framebuffer operation"
            );
            break;
        case GL_OUT_OF_MEMORY:
            pgWarning("OpenGL out of memory");
            break;
            /*
        case GL_STACK_UNDERFLOW:
            pgWarning("OpenGL stack underflow");
            break;
        case GL_STACK_OVERFLOW:
            pgWarning("OpenGL stack overflow");
            break;
            */
        default:
            pgWarning("OpenGL unknown error");
            break;
    }
}

/* 
 * Constructs, initializes, and returns a new
 * _TFGraphics object by value
 */
_TFGraphics _tfGraphicsMake(
    int graphicsWidth,
    int graphicsHeight
){
    _TFGraphics toRet = {0};
    toRet._graphicsWidth = graphicsWidth;
    toRet._graphicsHeight = graphicsHeight;

    /* set up VAO */
    glGenVertexArrays(1, &(toRet._vaoID));
    glBindVertexArray(toRet._vaoID);

    /* load shaders */
    toRet._programID = _loadShaders();

    return toRet;
}

//todo: temp function
void testDraw(_TFGraphics *graphicsPtr){
    /* test triangle */
    static const GLfloat vertexBufferData[] = {
       -1.0f, -1.0f, 0.0f,
       1.0f, -1.0f, 0.0f,
       0.0f,  1.0f, 0.0f,
    };
    /* set up vertex buffer */
    GLuint vertexBufferID;
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertexBufferData),
        vertexBufferData,
        GL_STATIC_DRAW
    );

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* use shaders */
    glUseProgram(graphicsPtr->_programID);

    /* first attribute buffer is vertices */
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    /* draw triangle starting from vertex 0 and with 3 total vertexes */
    glDrawArrays(GL_TRIANGLES, 0, 3);
    checkGLError();

    glDisableVertexAttribArray(0);
}

/* Frees the specified _TFGraphics */
void _tfGraphicsFree(_TFGraphics *graphicsPtr){
    if(graphicsPtr){
        /* clean up VAO */
        glDeleteBuffers(1, &(graphicsPtr->_vaoID));
	    glDeleteVertexArrays(
            1,
            &(graphicsPtr->_vaoID)
        );

	    glDeleteProgram(graphicsPtr->_programID);
    }
}