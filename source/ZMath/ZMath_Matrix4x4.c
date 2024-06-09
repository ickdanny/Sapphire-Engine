#include "ZMath_Matrix4x4.h"

#include <math.h>

/* 
 * Constructs and returns the 4x4 identity matrix 
 * by value
 */
Matrix4x4 matrix4x4MakeIdentity(){
    Matrix4x4 toRet = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    return toRet;
}

/* 
 * Constructs and returns a translation matrix
 * by value
 */
Matrix4x4 matrix4x4MakeTranslation(
    float x,
    float y,
    float z
){
    Matrix4x4 toRet = {
        1.0f, 0.0f, 0.0f, x,
        0.0f, 1.0f, 0.0f, y,
        0.0f, 0.0f, 1.0f, z,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    return toRet;
}

/*
 * Constructs and returns a rotation matrix about
 * the x axis by value
 */
Matrix4x4 matrix4x4MakeRotationX(float radians){
    float cosT = cosf(radians);
    float sinT = sinf(radians);
    Matrix4x4 toRet = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cosT, sinT, 0.0f,
        0.0f, -sinT, cosT, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    return toRet;
}

/*
 * Constructs and returns a rotation matrix about
 * the y axis by value
 */
Matrix4x4 matrix4x4MakeRotationY(float radians){
    float cosT = cosf(radians);
    float sinT = sinf(radians);
    Matrix4x4 toRet = {
        cosT, 0.0f, -sinT, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sinT, 0.0f, cosT, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    return toRet;
}

/*
 * Constructs and returns a rotation matrix about
 * the z axis by value
 */
Matrix4x4 matrix4x4MakeRotationZ(float radians){
    float cosT = cosf(radians);
    float sinT = sinf(radians);
    Matrix4x4 toRet = {
        cosT, -sinT, 0.0f, 0.0f,
        sinT, cosT, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    return toRet;
}

/* Constucts and returns a scaling matrix by value */
Matrix4x4 matrix4x4MakeScaling(
    float xScale,
    float yScale,
    float zScale
){
    Matrix4x4 toRet = {
        xScale, 0.0f, 0.0f, 0.0f,
        0.0f, yScale, 0.0f, 0.0f,
        0.0f, 0.0f, zScale, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    return toRet;
}

/* 
 * Returns the dot product of the ith row in the left
 * matrix with the jth column in the right matrix
 */
static float matrix4x4DotProduct(
    const Matrix4x4 *leftPtr,
    const Matrix4x4 *rightPtr,
    int row,
    int col
){
    return (leftPtr->matrix[row][0]
            * rightPtr->matrix[0][col])
        + (leftPtr->matrix[row][1]
            * rightPtr->matrix[1][col])
        + (leftPtr->matrix[row][2]
            * rightPtr->matrix[2][col])
        + (leftPtr->matrix[row][3]
            * rightPtr->matrix[3][col]);
}

/* 
 * Returns the result of multiplying the left matrix
 * by the right matrix
 */
Matrix4x4 matrix4x4Multiply(
    const Matrix4x4 *leftPtr,
    const Matrix4x4 *rightPtr
){
    Matrix4x4 toRet = {0};

    /* row column multiplication */
    for(int row = 0; row < 4; ++row){
        for(int col = 0; col < 4; ++col){
            toRet.matrix[row][col]
                = matrix4x4DotProduct(
                    leftPtr,
                    rightPtr,
                    row,
                    col
                );
        }
    }
    return toRet;
}

/* 
 * Returns the result of transposing the specified
 * matrix
 */
Matrix4x4 matrix4x4Transpose(
    const Matrix4x4 *matrixPtr
){
    Matrix4x4 toRet = {0};

    /* swap Aij with Aji*/
    for(int row = 0; row < 4; ++row){
        for(int col = 0; col < 4; ++col){
            toRet.matrix[row][col]
                = matrixPtr->matrix[col][row];
        }
    }
    return toRet;
}