#ifndef ZMATH_MATRIX4X4
#define ZMATH_MATRIX4X4

/* Represents a 4x4 matrix in row-major order */
typedef struct Matrix4x4{
    float matrix[4][4];
} Matrix4x4;

/* 
 * Constructs and returns the 4x4 identity matrix 
 * by value
 */
Matrix4x4 matrix4x4MakeIdentity();

/* 
 * Constructs and returns a translation matrix
 * by value
 */
Matrix4x4 matrix4x4MakeTranslation(
    float x,
    float y,
    float z
);

/*
 * Constructs and returns a rotation matrix about
 * the x axis by value
 */
Matrix4x4 matrix4x4MakeRotationX(float radians);

/*
 * Constructs and returns a rotation matrix about
 * the y axis by value
 */
Matrix4x4 matrix4x4MakeRotationY(float radians);

/*
 * Constructs and returns a rotation matrix about
 * the z axis by value
 */
Matrix4x4 matrix4x4MakeRotationZ(float radians);

/* Constucts and returns a scaling matrix by value */
Matrix4x4 matrix4x4MakeScaling(
    float xScale,
    float yScale,
    float zScale
);

/* 
 * Constructs and returns a scaling matrix along the
 * x axis by value
 */
#define matrix4x4MakeScalingX(XSCALE) \
    matrix4x4MakeScaling(XSCALE, 1.0f, 1.0f)

/* 
 * Constructs and returns a scaling matrix along the
 * y axis by value
 */
#define matrix4x4MakeScalingY(YSCALE) \
    matrix4x4MakeScaling(1.0f, YSCALE, 1.0f)

/* 
 * Constructs and returns a scaling matrix along the
 * z axis by value
 */
#define matrix4x4MakeScalingZ(ZSCALE) \
    matrix4x4MakeScaling(1.0f, 1.0f, ZSCALE)

/* 
 * Returns the result of multiplying the left matrix
 * by the right matrix
 */
Matrix4x4 matrix4x4Multiply(
    const Matrix4x4 *leftPtr,
    const Matrix4x4 *rightPtr
);

/* 
 * Returns the result of transposing the specified
 * matrix
 */
Matrix4x4 matrix4x4Transpose(
    const Matrix4x4 *matrixPtr
);

#endif