#include "_Trifecta_Shaders.h"

#include "Constructure.h"

/* source code for the vertex shader */
static const char *vertexShaderSource = \
""
;

/* source code for the fragment shader */
static const char *fragmentShaderSource = \
""
;

/* Loads the shaders and returns the programID */
GLuint _loadShaders(){
    /* debugging variables */
    GLint status = GL_FALSE;
    int logLength = 0;

    /* create empty shaders */
	GLuint vertexShaderID = glCreateShader(
        GL_VERTEX_SHADER
    );
	GLuint fragmentShaderID = glCreateShader(
        GL_FRAGMENT_SHADER
    );

    /* compile vertex shader */
    glShaderSource(
        vertexShaderID,
        1, /* count */
        &vertexShaderSource,
        NULL /* length */
    );
	glCompileShader(vertexShaderID);

    /* make sure compilation worked */
    glGetShaderiv(
        vertexShaderID,
        GL_COMPILE_STATUS,
        &status
    );
    if(!status){
	    glGetShaderiv(
            vertexShaderID,
            GL_INFO_LOG_LENGTH,
            &logLength
        );
	    if(logLength > 0){
            String errorMsg = stringMakeAndReserve(
                logLength + 1
            );
	    	glGetShaderInfoLog(
                vertexShaderID,
                logLength,
                NULL,
                errorMsg._ptr
            );
            pgError(errorMsg._ptr);
	    }
    }

    /* compile fragment shader */
    glShaderSource(
        fragmentShaderID,
        1, /* count */
        &fragmentShaderSource,
        NULL /* length */
    );
	glCompileShader(fragmentShaderID);

    /* make sure compilation worked */
    glGetShaderiv(
        fragmentShaderID,
        GL_COMPILE_STATUS,
        &status
    );
    if(!status){
	    glGetShaderiv(
            fragmentShaderID,
            GL_INFO_LOG_LENGTH,
            &logLength
        );
	    if(logLength > 0){
            String errorMsg = stringMakeAndReserve(
                logLength + 1
            );
	    	glGetShaderInfoLog(
                fragmentShaderID,
                logLength,
                NULL,
                errorMsg._ptr
            );
            pgError(errorMsg._ptr);
	    }
    }

    /* link program */
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

    /* make sure linking worked */
    glGetProgramiv(
        programID,
        GL_LINK_STATUS,
        &status
    );
    if(!status){
	    glGetProgramiv(
            programID,
            GL_INFO_LOG_LENGTH,
            &logLength
        );
	    if(logLength > 0){
            String errorMsg = stringMakeAndReserve(
                logLength + 1
            );
	    	glGetProgramInfoLog(
                programID,
                logLength,
                NULL,
                errorMsg._ptr
            );
            pgError(errorMsg._ptr);
	    }
    }

    /* clean up */
    glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

    return programID;
}