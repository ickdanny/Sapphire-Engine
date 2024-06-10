#include "_Trifecta_Shaders.h"

#include "Constructure.h"

/* source code for the vertex shader */
static const char *vertexShaderSource =
    "#version 330 core \n"
    "layout(location = 0) in vec3 vertexPos; \n"
    "layout(location = 1) in vec2 vertexTexCoords; \n"
    "out vec2 texCoords; \n"
    "uniform mat4 transform; \n"
    "void main(){ \n"
        "gl_Position.xyz = vertexPos; \n"
        "gl_Position.w = 1.0; \n"
        "gl_Position = transform * gl_Position; \n"
        "texCoords = vertexTexCoords; \n"
    "} \n"
;

/* source code for the fragment shader */
static const char *fragmentShaderSource =
    "#version 330 core \n"
    "in vec2 texCoords; \n"
    "out vec4 color; \n"
    "uniform sampler2D sampler; \n"
    "void main(){ \n"
        "color = texture(sampler, texCoords); \n"
        "if(color.a < 1.0){ \n"
            "discard; \n"
        "} \n"
    "} \n"
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
            pgWarning("error compiling vertex shader");
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
            pgWarning("error compiling frag shader");
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