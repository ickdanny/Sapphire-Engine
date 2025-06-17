#ifndef TRIFECTA_INPUT
#define TRIFECTA_INPUT

#include "Constructure.h"

#include "Trifecta_Window.h"

/* Identifies a specific key on the keyboard */
typedef enum TFKeyValue{
    /* alphabet */
	tf_a,
	tf_b,
	tf_c,
	tf_d,
	tf_e,
	tf_f,
	tf_g,
	tf_h,
	tf_i,
	tf_j,
	tf_k,
	tf_l,
	tf_m,
	tf_n,
	tf_o,
	tf_p,
	tf_q,
	tf_r,
	tf_s,
	tf_t,
	tf_u,
	tf_v,
	tf_w,
	tf_x,
	tf_y,
	tf_z,

    /* numbers*/
	tf_0,
	tf_1,
	tf_2,
	tf_3,
	tf_4,
	tf_5,
	tf_6,
	tf_7,
	tf_8,
	tf_9,
	tf_numpad0,
	tf_numpad1,
	tf_numpad2,
	tf_numpad3,
	tf_numpad4,
	tf_numpad5,
	tf_numpad6,
	tf_numpad7,
	tf_numpad8,
	tf_numpad9,
	
    /* arrow keys */
	tf_left,
	tf_right,
	tf_up,
	tf_down,

	/* special */
	tf_escape,
	tf_space,
	tf_backTick,
	tf_minus,
	tf_equal,
	tf_backSlash,
	tf_backSpace,
	tf_enter,
	tf_leftBracket,
	tf_rightBracket,
	tf_semicolon,
	tf_quote,
	tf_comma,
	tf_period,
	tf_slash,
	tf_tab,
	tf_shift,
	tf_control,
	tf_alt,
	
    /* meta */
	tf_undefined,
	tf_numKeys
} TFKeyValue;

/* Identifies the possible states a key can be in */
typedef enum TFKeyState{
    tf_stateUp,
    tf_stateDown,
    tf_statePress,
    tf_stateRelease,
    tf_stateLocked   /* means key cannot be read */
} TFKeyState;

/*
 * Stores the input state of all keys defined by
 * TFKeyValue
 */
typedef struct TFKeyTable{
    /* the key table is indexed by key Id */
    Array _keyTable;
} TFKeyTable;

/* 
 * Constructs and returns a new TFKeyTable by value
 * and registers it to start receiving input signals
 * from the specified TFWindow
 */
TFKeyTable tfKeyTableMake(TFWindow *windowPtr);

/* Turns all keys off in the specified TFKeyTable */
void tfKeyTableAllKeysOff(TFKeyTable *keyTablePtr);

/*
 * Gets the state of the specified key in the given
 * TFKeyTable
 */
TFKeyState tfKeyTableGetState(
    TFKeyTable *keyTablePtr,
    TFKeyValue key
);

/* 
 * Locks the specified key from being read in the
 * given TFKeyTable until the next tick
 */
void tfKeyTableLock(
    TFKeyTable *keyTablePtr,
    TFKeyValue key
);

/*
 * Locks all keys from being read in the given
 * TFKeyTable until the next tick
 */
void tfKeyTableLockAll(TFKeyTable *keyTablePtr);

/* 
 * Returns true if the specified key is locked
 * in the given TFKeyTable, false otherwise
 */
bool tfKeyTableIsLocked(
    TFKeyTable *keyTablePtr,
    TFKeyValue key
);

/*
 * Steps the tick for the specified TFKeyTable by
 * unlocking all keys and advancing the key state
 */
void tfKeyTableStepTick(TFKeyTable *keyTablePtr);

/* 
 * Frees the memory associated with the specified
 * TFKeyTable
 */
void tfKeyTableFree(TFKeyTable *keyTablePtr);

#endif