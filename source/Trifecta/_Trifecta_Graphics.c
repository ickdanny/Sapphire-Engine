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
            pgError("OpenGL invalid enum");
            break;
        case GL_INVALID_VALUE:
            pgError("OpenGL invalid value");
            break;
        case GL_INVALID_OPERATION:
            pgError("OpenGL invalid operation");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            pgError(
                "OpenGL invalid framebuffer operation"
            );
            break;
        case GL_OUT_OF_MEMORY:
            pgError("OpenGL out of memory");
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
            pgError("OpenGL unknown error");
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
    glUseProgram(toRet._programID);

    /* load the vertex buffer for quads */
    static const GLfloat quadVertices[] = {
        /* first triangle */
        -1.0f, -1.0f, 0.0f,  /* bottom left */
        -1.0f, 1.0f, 0.0f,   /* top left */
        1.0f, 1.0f, 0.0f,    /* top right */
        /* second triangle */
        -1.0f, -1.0f, 0.0f,  /* bottom left */
        1.0f, 1.0f, 0.0f,    /* top right */
        1.0f, -1.0f, 0.0f,   /* bottom right*/
    };
    /* set up vertex buffer */
    GLuint vertexBufferID;
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(quadVertices),
        quadVertices,
        GL_STATIC_DRAW
    );
    /* first attribute buffer is vertices */
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(
        0,          /* use attribute 0 */
        3,          /* num components per vertex */
        GL_FLOAT,   /* type of components */
        GL_FALSE,   /* not normalized */
        0,          /* stride 0: tightly packed */
        (void*)0    /* array buffer offset */
    );

    return toRet;
}

/* Signals OpenGL to draw a quad */
static void drawQuad(){
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_TRIANGLES, 3, 3);  
}

//todo: temp function
void testDraw(_TFGraphics *graphicsPtr){
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    drawQuad();
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

        /* disable vertex buffer */
        glDisableVertexAttribArray(0);

	    glDeleteProgram(graphicsPtr->_programID);
    }
}