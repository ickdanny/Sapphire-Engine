#include "_Trifecta_Graphics.h"

#include "PGUtil.h"

#include "_Trifecta_Shaders.h"

/* todo: cmt out
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
            //todo: cmt out
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

    /* enable depth buffer */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* get transform ID */
    toRet._transformID = glGetUniformLocation(
        toRet._programID,
        "transform"
    );

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

/* Normalizes a point to within [-1.0, 1.0] */
static Point2D normalizePoint(
    Point2D point,
    float graphicsWidth,
    float graphicsHeight
){
    /* point is copied by value */
    point.x /= graphicsWidth;
    point.y /= graphicsHeight;
    point.x *= 2;
    point.y *= 2;
    point.x -= 1;
    point.y -= 1;
    return point;
}

/* Constructs a transformation matrix */
static Matrix4x4 makeTransformMatrix(
    float graphicsWidth,
    float graphicsHeight,
    Point2D preOffsetCenter,
    const TFSpriteInstruction *spriteInstrPtr,
    float quadWidth,
    float quadHeight
){
    float aspect = graphicsWidth / graphicsHeight;
    float widthScale = quadWidth / graphicsWidth;
    float heightScale = quadHeight / graphicsHeight;

    Point2D offsetCenter = point2DAddVector2D(
        preOffsetCenter, spriteInstrPtr->offset
    );

    Point2D normalizedOffsetCenter = normalizePoint(
        offsetCenter,
        graphicsWidth,
        graphicsHeight
    );

    //todo: scale depth?
    /*
    static constexpr float depthRange{
			SpriteDrawInstruction::maxDepth - SpriteDrawInstruction::minDepth + 1
		};
		float depthShift{ static_cast<float>(spriteDrawInstruction.getDepth()) / depthRange };
        */

    /* reverse aspect correction */
    Matrix4x4 scalingMatrix = matrix4x4MakeScaling(
        aspect * widthScale * spriteInstrPtr->scale,
        heightScale * spriteInstrPtr->scale,
        1.0f /* z scale */
    );

    Matrix4x4 rotationMatrix = matrix4x4MakeRotationZ(
        toRadians(spriteInstrPtr->rotation)
    );

    /* aspect corection */
    Matrix4x4 aspectMatrix = matrix4x4MakeScaling(
        1.0f / aspect,
        1.0f,
        1.0f
    );

    Matrix4x4 translationMatrix
        = matrix4x4MakeTranslation(
            normalizedOffsetCenter.x,
            normalizedOffsetCenter.y,
            spriteInstrPtr->depth
        );
    
    /* 
     * first apply scaling, then rotation, then aspect,
     * then translation via left multiplication
     */
    Matrix4x4 transformMatrix = matrix4x4Multiply(
        &rotationMatrix,
        &scalingMatrix
    );
    transformMatrix = matrix4x4Multiply(
        &aspectMatrix,
        &transformMatrix
    );
    transformMatrix = matrix4x4Multiply(
        &translationMatrix,
        &transformMatrix
    );

    return transformMatrix;
}

//todo: temp function
void testDraw(_TFGraphics *graphicsPtr){
    TFSprite sprite = parseBitmapFile("test2.bmp");

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    static float rotation = 0.0f;
    rotation += 1.0f;
    Vector2D offset = {0.0f, 0.0f};
    TFSpriteInstruction spriteInstr
        = tfSpriteInstructionMake(
            &sprite,
            0,
            offset,
            rotation,
            sinf(toRadians(rotation))
        );
    Point2D center = {
        graphicsPtr->_graphicsWidth / 2,
        graphicsPtr->_graphicsHeight / 2
    };
    _tfGraphicsDrawSprite(
        graphicsPtr,
        center,
        &spriteInstr
    );
}

//todo: batched rendering?

/* Draws a sprite with the specified _TFGraphics */
void _tfGraphicsDrawSprite(
    _TFGraphics *graphicsPtr,
    Point2D preOffsetCenter,
    const TFSpriteInstruction *spriteInstrPtr
){
    assertNotNull(
        spriteInstrPtr,
        "error null sprite instruction"
    );
    assertNotNull(
        spriteInstrPtr->spritePtr,
        "error null sprite"
    );

    /* draw nothing if scale is 0 */
    if(spriteInstrPtr->scale == 0.0f){
        return;
    }

    //todo: send texture (?)
    //todo: send transform (?)
    Matrix4x4 transformMatrix = makeTransformMatrix(
        graphicsPtr->_graphicsWidth,
        graphicsPtr->_graphicsHeight,
        preOffsetCenter,
        spriteInstrPtr,
        spriteInstrPtr->spritePtr->width,
        spriteInstrPtr->spritePtr->height
    );
    glUniformMatrix4fv(
        graphicsPtr->_transformID,
        1,
        GL_TRUE,
        &(transformMatrix.matrix[0][0])
    );
    
    drawQuad();
}

/*
 * Draws a portion of a sprite with the specified
 * _TFGraphics
 */
void _tfGraphicsDrawSubSprite(
    _TFGraphics *graphicsPtr,
    Point2D preOffsetCenter,
    const TFSpriteInstruction *spriteInstrPtr,
    const Rectangle *srcRectPtr
){
    //todo
}

/* 
 * Draws a tiled sprite with the specified _TFGraphics
 */
void _tfGraphicsDrawTileSprite(
    _TFGraphics *graphicsPtr,
    const Rectangle *drawRectPtr,
    const TFSpriteInstruction *spriteInstrPtr,
    Point2D pixelOffset
){
    //todo
}

/* Draws text with the specified _TFGraphics */
void _tfGraphicsDrawText(
    _TFGraphics *graphicsPtr,
    Point2D pos,
    const WideString *wideStringPtr,
    int rightBound,
    TFGlyphMap *glyphMapPtr
){
    //todo
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