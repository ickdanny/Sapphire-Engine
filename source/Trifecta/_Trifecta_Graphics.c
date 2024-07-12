#include "_Trifecta_Graphics.h"

#include "PGUtil.h"

#include "_Trifecta_Shaders.h"
#include "Config.h"

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
        default:
            pgError("OpenGL unknown error");
            break;
    }
}

/* 
 * Updates the texCoord buffer to draw the specified
 * Rectangle
 */
static void _tfGraphicsUpdateTexCoordBuffer(
    const _TFGraphics *graphicsPtr,
    Rectangle texCoordRect
){
    float uLow = texCoordRect.x;
    float uHigh = uLow + texCoordRect.width;
    float vLow = texCoordRect.y;
    float vHigh = vLow + texCoordRect.height;
    GLfloat quadTexCoords[] = {
        /* first triangle */
        uLow, vLow,     /* bottom left */
        uLow, vHigh,    /* top left */
        uHigh, vHigh,   /* top right */
        /* second triangle */
        uLow, vLow,     /* bottom left */
        uHigh, vHigh,   /* top right */
        uHigh, vLow,    /* bottom right*/
    };
    /* send the texCoord buffer */
    glBindBuffer(
        GL_ARRAY_BUFFER,
        graphicsPtr->_texCoordBufferID
    );
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(quadTexCoords),
        quadTexCoords,
        GL_STATIC_DRAW
    );
}

/* The rectangle which represents the full texture */
static const Rectangle fullTexRect = {
    0.0f,
    0.0f,
    1.0f,
    1.0f
};

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

    /* get sampler ID */
    toRet._samplerID = glGetUniformLocation(
        toRet._programID,
        "sampler"
    );
    /* set sampler to use texture 0 */
    glUniform1i(toRet._samplerID, 0);

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
    glGenBuffers(1, &toRet._vertexBufferID);
    glBindBuffer(
        GL_ARRAY_BUFFER,
        toRet._vertexBufferID
    );
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(quadVertices),
        quadVertices,
        GL_STATIC_DRAW
    );
    /* first attribute buffer is vertices */
    glEnableVertexAttribArray(0);
    glBindBuffer(
        GL_ARRAY_BUFFER,
        toRet._vertexBufferID
    );
    glVertexAttribPointer(
        0,          /* use attribute 0 */
        3,          /* num components per vertex */
        GL_FLOAT,   /* type of components */
        GL_FALSE,   /* not normalized */
        0,          /* stride 0: tightly packed */
        (void*)0    /* array buffer offset */
    );

    /* load the texCoord buffer for quads */
    /* set up texCoord buffer */
    glGenBuffers(1, &toRet._texCoordBufferID);
    _tfGraphicsUpdateTexCoordBuffer(
        &toRet,
        fullTexRect
    );
    
    /* second attribute buffer is texCoords */
    glEnableVertexAttribArray(1);
    glBindBuffer(
        GL_ARRAY_BUFFER,
        toRet._texCoordBufferID
    );
    glVertexAttribPointer(
        1,          /* use attribute 1 */
        2,          /* num components per vertex */
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
    
    static const float depthRange
        = config_maxDepth - config_minDepth;
	float depthShift
        = spriteInstrPtr->depth / depthRange;

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
            depthShift
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

//todo: instanced rendering is possible optimization

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

    /* send texture to OpenGL (bind to 0) */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(
        GL_TEXTURE_2D,
        spriteInstrPtr->spritePtr->_textureID
    );

    /* send transform to OpenGL */
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
 * _TFGraphics; scaling and rotation will apply to
 * the new center of the sprite
 */
void _tfGraphicsDrawSubSprite(
    _TFGraphics *graphicsPtr,
    Point2D preOffsetCenter,
    const TFSpriteInstruction *spriteInstrPtr,
    const Rectangle *srcRectPtr
){
    /* if width or height are small, draw nothing */
	if(srcRectPtr->width < 0.5f
        || srcRectPtr->height < 0.5f
    ){
		return;
	}
	
    /* change tex coords */
	float fullWidth
        = spriteInstrPtr->spritePtr->width;
	float fullHeight
        = spriteInstrPtr->spritePtr->height;
    Rectangle texCoordRect = {0};
	texCoordRect.x = srcRectPtr->x / fullWidth;
	texCoordRect.y = srcRectPtr->y / fullHeight;

    texCoordRect.width = srcRectPtr->width / fullWidth;
    texCoordRect.height
        = srcRectPtr->height / fullHeight;

    /* send sub texCoords */
    _tfGraphicsUpdateTexCoordBuffer(
        graphicsPtr,
        texCoordRect
    );
	
	/* find the new center for the quad */
	Point2D subCenter = {
		srcRectPtr->x + srcRectPtr->width / 2.0f,
		srcRectPtr->y + srcRectPtr->height / 2.0f
	};
	Point2D fullCenter = {
        fullWidth / 2.0f,
        fullHeight / 2.0f
    };
    Vector2D subOffset = vector2DFromAToB(
        fullCenter,
        subCenter
    );
    Point2D newPreOffsetCenter = point2DAddVector2D(
        preOffsetCenter,
        subOffset
    );
	
    /* draw sprite */
    /* send texture to OpenGL (bind to 0) */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(
        GL_TEXTURE_2D,
        spriteInstrPtr->spritePtr->_textureID
    );
    /* send transform to OpenGL */
    Matrix4x4 transformMatrix = makeTransformMatrix(
        graphicsPtr->_graphicsWidth,
        graphicsPtr->_graphicsHeight,
        newPreOffsetCenter,
        spriteInstrPtr,
        srcRectPtr->width,
        srcRectPtr->height
    );
    glUniformMatrix4fv(
        graphicsPtr->_transformID,
        1,
        GL_TRUE,
        &(transformMatrix.matrix[0][0])
    );
	drawQuad();
	
	/* restore default texCoords */
    _tfGraphicsUpdateTexCoordBuffer(
        graphicsPtr,
        fullTexRect
    );
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
    /* 
     * change texture coordinates to tile the draw
     * rectangle
     */
	float tileWidth = spriteInstrPtr->spritePtr->width;
	float tileHeight
        = spriteInstrPtr->spritePtr->height;
    Rectangle texCoordRect = {0};
    texCoordRect.x = pixelOffset.x / tileWidth;
    texCoordRect.y = pixelOffset.y / tileHeight;
    texCoordRect.width
        = drawRectPtr->width / tileWidth;
    texCoordRect.height
        = drawRectPtr->height / tileHeight;
    
    /* send tiled texCoords */
	_tfGraphicsUpdateTexCoordBuffer(
        graphicsPtr,
        texCoordRect
    );
	
    /* draw sprite covering draw rectangle */
	Point2D preOffsetCenter = {
		drawRectPtr->x + drawRectPtr->width / 2.0f,
		drawRectPtr->y + drawRectPtr->height / 2.0f
	};
	
    /* draw sprite */
	/* send texture to OpenGL (bind to 0) */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(
        GL_TEXTURE_2D,
        spriteInstrPtr->spritePtr->_textureID
    );
    /* send transform to OpenGL */
    Matrix4x4 transformMatrix = makeTransformMatrix(
        graphicsPtr->_graphicsWidth,
        graphicsPtr->_graphicsHeight,
        preOffsetCenter,
        spriteInstrPtr,
        drawRectPtr->width,
        drawRectPtr->height
    );
    glUniformMatrix4fv(
        graphicsPtr->_transformID,
        1,
        GL_TRUE,
        &(transformMatrix.matrix[0][0])
    );
	drawQuad();
	
    /* restore default texCoords */
    _tfGraphicsUpdateTexCoordBuffer(
        graphicsPtr,
        fullTexRect
    );
}

/* 
 * Draws text (as a WideString) with the specified
 * _TFGraphics
 */
void _tfGraphicsDrawText(
    _TFGraphics *graphicsPtr,
    Point2D pos,
    const WideString *textPtr,
    int rightBound,
    TFGlyphMap *glyphMapPtr
){
    /* bail if string is empty */
    if(wideStringIsEmpty(textPtr)){
        return;
    }

    /* bail if right bound is invalid */
    int startX = (int)pos.x;
    if(startX >= rightBound){
        pgWarning(
            "cannot draw text: startX >= rightBound"
        );
        return;
    }

    int currentX = startX;
    int currentY = (int)pos.y;
    int horizontalSpacing
        = glyphMapPtr->horizontalSpacing;
    int verticalSpacing
        = glyphMapPtr->verticalSpacing;
    
    /* helper function to calculate next coordinate */
    #define stepCurrentCoordinates() \
        do{ \
            currentX += horizontalSpacing; \
		    if(currentX >= rightBound){ \
		    	currentX = startX; \
		    	currentY -= verticalSpacing; \
		    } \
        } while(false);
	
    wchar_t currentChar = 0;
	Point2D currentPos = {0};
    int textLength = textPtr->length;
	for(int stringPos = 0;
        stringPos < textLength;
        ++stringPos
    ){
		currentPos.x = (float)(currentX);
		currentPos.y = (float)(currentY);
        currentChar = wideStringCharAt(
            textPtr,
            stringPos
        );
		switch(currentChar){
			case L' ':  /* space */
				stepCurrentCoordinates();
				continue;
			case L'\t': /* tab */
				stepCurrentCoordinates();
				stepCurrentCoordinates();
				stepCurrentCoordinates();
				continue;
			case L'\n':	/* new line */
				currentX = startX;
				currentY += verticalSpacing;
			default:    /* all other chars*/
				stepCurrentCoordinates();
                _tfGraphicsDrawSprite(
                    graphicsPtr,
                    currentPos,
                    tfGlyphMapGet(
                        glyphMapPtr,
                        currentChar
                    )
                );
		} /* switch */
	} /* loop */
    
    #undef stepCurrentCoordinates
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

        /* clean up buffers */
        glDisableVertexAttribArray(0);
        glDeleteBuffers(
            1,
            &(graphicsPtr->_vertexBufferID)
        );
        glDisableVertexAttribArray(1);
        glDeleteBuffers(
            1,
            &(graphicsPtr->_texCoordBufferID)
        );

	    glDeleteProgram(graphicsPtr->_programID);
    }
}