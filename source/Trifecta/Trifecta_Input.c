#include "Trifecta_Input.h"

typedef uint_fast8_t DataType;

#define lockMask ((DataType)0x80)
#define currentTickMask ((DataType)0x01)
#define prevTickMask ((DataType)0x02)
#define ticksMask \
    ((DataType)(currentTickMask | prevTickMask))

/* 
 * A global pointer to the start of the _keyTable
 * array which the callback function will use; should
 * be registered to the new TFKeyTable when it is
 * constructed and unregistered when it is freed
 */
static DataType *keyDataPtr;

/* Translates a GLFW key value to a TFKeyValue */
static TFKeyValue translateGLFWKey(int glfwKey){
    #define caseMacro(TF, GLFW) \
        case GLFW_KEY_##GLFW: \
            return tf_##TF
    switch(glfwKey){
        caseMacro(a, A);
        caseMacro(b, B);
        caseMacro(c, C);
        caseMacro(d, D);
        caseMacro(e, E);
        caseMacro(f, F);
        caseMacro(g, G);
        caseMacro(h, H);
        caseMacro(i, I);
        caseMacro(j, J);
        caseMacro(k, K);
        caseMacro(l, L);
        caseMacro(m, M);
        caseMacro(n, N);
        caseMacro(o, O);
        caseMacro(p, P);
        caseMacro(q, Q);
        caseMacro(r, R);
        caseMacro(s, S);
        caseMacro(t, T);
        caseMacro(u, U);
        caseMacro(v, V);
        caseMacro(w, W);
        caseMacro(x, X);
        caseMacro(y, Y);
        caseMacro(z, Z);
        caseMacro(0, 0);
        caseMacro(1, 1);
        caseMacro(2, 2);
        caseMacro(3, 3);
        caseMacro(4, 4);
        caseMacro(5, 5);
        caseMacro(6, 6);
        caseMacro(7, 7);
        caseMacro(8, 8);
        caseMacro(9, 9);
        caseMacro(numpad0, KP_0);
        caseMacro(numpad1, KP_1);
        caseMacro(numpad2, KP_2);
        caseMacro(numpad3, KP_3);
        caseMacro(numpad4, KP_4);
        caseMacro(numpad5, KP_5);
        caseMacro(numpad6, KP_6);
        caseMacro(numpad7, KP_7);
        caseMacro(numpad8, KP_8);
        caseMacro(numpad9, KP_9);
        caseMacro(left, LEFT);
        caseMacro(right, RIGHT);
        caseMacro(up, UP);
        caseMacro(down, DOWN);
        caseMacro(escape, ESCAPE);
        caseMacro(space, SPACE);
        caseMacro(backTick, GRAVE_ACCENT);
	    caseMacro(minus, MINUS);
	    caseMacro(equal, EQUAL);
	    caseMacro(backSlash, BACKSLASH);
	    caseMacro(backSpace, BACKSPACE);
	    caseMacro(enter, ENTER);
	    caseMacro(leftBracket, LEFT_BRACKET);
	    caseMacro(rightBracket, RIGHT_BRACKET);
	    caseMacro(semicolon, SEMICOLON);
	    caseMacro(quote, APOSTROPHE);
	    caseMacro(comma, COMMA);
	    caseMacro(period, PERIOD);
	    caseMacro(slash, SLASH);
	    caseMacro(tab, TAB);
	    caseMacro(shift, LEFT_SHIFT);
        caseMacro(shift, RIGHT_SHIFT);
	    caseMacro(control, LEFT_CONTROL);
        caseMacro(control, RIGHT_CONTROL);
	    caseMacro(alt, LEFT_ALT);
        caseMacro(alt, RIGHT_ALT);
        #undef caseMacro

        default:
            return tf_undefined;
    }
}

/* 
 * The callback function that sends input to the
 * currently registered TFKeyTable
 */
static void keyCallback(
    GLFWwindow *windowPtr,
    int glfwKey,
    int scancode,
    int action,
    int modifiers
){
    /* bail if no key table is registered */
    if(!keyDataPtr){
        return;
    }

    TFKeyValue tfKey = translateGLFWKey(glfwKey);
    switch(action){
        /* if pressed, set current tick */
        case GLFW_PRESS:
            keyDataPtr[tfKey] |= currentTickMask;
            break;
        /* if released, unset current tick */
        case GLFW_RELEASE:
            keyDataPtr[tfKey] &= (~currentTickMask);
            break;
        /* ignore repeat */
    }
}

/*
 * Turns off the key by modifying the underlying
 * data type
 */
static void turnKeyOff(DataType *dataPtr){
    (*dataPtr) = (*dataPtr & (~currentTickMask));
}

/* 
 * The callback function that turns all keys off when
 * the window loses focus
 */
static void focusCallback(
    GLFWwindow *windowPtr,
    int focused
){
    /* bail if no key table is registered */
    if(!keyDataPtr){
        return;
    }

    /* if window lost focus, turn all keys off */
    if(focused == GLFW_FALSE){
        for(size_t i = 0; i < tf_numKeys; ++i){
            turnKeyOff(&(keyDataPtr[i]));
        }
    }
}

/* 
 * Constructs and returns a new TFKeyTable by value
 * and registers it to start receiving input signals
 * from the specified TFWindow
 */
TFKeyTable tfKeyTableMake(TFWindow *windowPtr){
    TFKeyTable toRet = {0};
    toRet._keyTable = arrayMake(DataType, tf_numKeys);

    /* register the key data */
    keyDataPtr = arrayGetPtr(
        DataType,
        &(toRet._keyTable),
        0
    );

    /* register key callback with the window */
    glfwSetKeyCallback(
        windowPtr->_windowPtr,
        keyCallback
    );
    /* register focus callback with the window */
    glfwSetWindowFocusCallback(
        windowPtr->_windowPtr,
        focusCallback
    );

    return toRet;
}

/* Turns all keys off in the specified TFKeyTable */
void tfKeyTableAllKeysOff(TFKeyTable *keyTablePtr){
    arrayApply(
        DataType,
        &(keyTablePtr->_keyTable),
        turnKeyOff
    );
}

/*
 * Gets the state of the specified key in the given
 * TFKeyTable
 */
TFKeyState tfKeyTableGetState(
    TFKeyTable *keyTablePtr,
    TFKeyValue key
){
    DataType data = arrayGet(
        DataType,
        &(keyTablePtr->_keyTable),
        key
    );

    /* check for lock state */
    if(data & lockMask){
        return tf_stateLocked;
    }

    /* switch on the data */
    data &= ticksMask;
    switch(data){
        /* only current tick means just pressed */
        case currentTickMask:
            return tf_statePress;
        /* only prev tick means just released */
        case prevTickMask:
            return tf_stateRelease;
        /* both ticks means down */
        case ticksMask:
            return tf_stateDown;
        /* no bits set means up */
        case 0x00:
            return tf_stateUp;
        /* error if some other case */
        default:
            pgError("unexpected state in input table");
            return tf_stateLocked;
    }
}

/* 
 * Locks the key by modifying the underlying data type
 */
static void lockKey(DataType *dataPtr){
    (*dataPtr) = (*dataPtr | lockMask);
}

/* 
 * Locks the specified key from being read in the
 * given TFKeyTable until the next tick
 */
void tfKeyTableLock(
    TFKeyTable *keyTablePtr,
    TFKeyValue key
){
    lockKey(arrayGetPtr(
        DataType,
        &(keyTablePtr->_keyTable),
        key
    ));
}

/*
 * Locks all keys from being read in the given
 * TFKeyTable until the next tick
 */
void tfKeyTableLockAll(TFKeyTable *keyTablePtr){
    arrayApply(
        DataType,
        &(keyTablePtr->_keyTable),
        lockKey
    );
}

/* 
 * Returns true if the specified key is locked
 * in the given TFKeyTable, false otherwise
 */
bool tfKeyTableIsLocked(
    TFKeyTable *keyTablePtr,
    TFKeyValue key
){
    DataType data = arrayGet(
        DataType,
        &(keyTablePtr->_keyTable),
        key
    );

    /* return true if lock bit is set */
    return data & lockMask;
}

/* 
 * Steps the key by modifying the underlying data type
 */
static void stepKey(DataType *dataPtr){
    DataType data = *dataPtr;
    /* unset the prev tick */
    data &= ~prevTickMask;
    /* copy current to prev tick */
    data |= ((data & currentTickMask) << 1);
    /* unlock key */
    data &= ~lockMask;

    *dataPtr = data;
}

/*
 * Steps the tick for the specified TFKeyTable by
 * unlocking all keys and advancing the key state
 */
void tfKeyTableStepTick(TFKeyTable *keyTablePtr){
    arrayApply(
        DataType,
        &(keyTablePtr->_keyTable),
        stepKey
    );
}

/* 
 * Frees the memory associated with the specified
 * TFKeyTable
 */
void tfKeyTableFree(TFKeyTable *keyTablePtr){
    /* unregister the key data ptr */
    keyDataPtr = NULL;

    /* free the array */
    arrayFree(DataType, &(keyTablePtr->_keyTable));

    /* don't bother unregistering the callbacks */
}