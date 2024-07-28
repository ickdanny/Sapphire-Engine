#include "DialogueSystem.h"

#define textRightBound 230

/*
 * Sets the image of the specified entity to the sprite
 * specified by the given id
 */
static void setImage(
    Game *gamePtr,
    Scene *scenePtr,
    WindEntity handle,
    String *spriteIDPtr
){
    SpriteInstruction spriteInstr
        = tfSpriteInstructionMakeSimple(
            resourcesGetSprite(
                gamePtr->resourcesPtr,
                spriteIDPtr
            ),
            config_foregroundDepth,
            ((Vector2D){0})
        );
    if(!(spriteInstr.spritePtr)){
        pgWarning(spriteIDPtr->_ptr);
        pgError(
            "Failed to get sprite for dialogue; "
            SRC_LOCATION
        );
    }
    windWorldHandleSetComponent(SpriteInstruction,
        &(scenePtr->ecsWorld),
        handle,
        &spriteInstr
    );
}

/*
 * Sets the text of the specified entity to the given
 * string
 */
static void setText(
    Scene *scenePtr,
    WindEntity handle,
    String *textPtr
){
    TextInstruction textInstr = {0};
    textInstr.text = wideStringMakeAndReserve(
        textPtr->length
    );
    for(size_t i = 0; i < textPtr->length; ++i){
        wideStringPushBack(
            &(textInstr.text),
            stringCharAt(textPtr, i)
        );
    }
    textInstr.rightBound = textRightBound;

    windWorldHandleSetComponent(TextInstruction,
        &(scenePtr->ecsWorld),
        handle,
        &textInstr
    );
}

/*
 * Handles a single dialogue command; returns true if
 * more commands should be handled afterwards, false
 * otherwise
 */
static bool handleNextCommand(
    Game *gamePtr,
    Scene *scenePtr
){
    Dialogue *dialoguePtr
        = scenePtr->messages.dialogueData.dialoguePtr;
    /* error if no dialogue ptr */
    assertNotNull(
        dialoguePtr,
        "failed to get dialogue from scene; "
        SRC_LOCATION
    );
    int *indexPtr = &(scenePtr->messages.dialogueData
        .nextDialogueIndex);

    /* bail if index invalid */
    if(*indexPtr >= dialoguePtr->instructionList.size){
        return false;
    }

    DialogueInstruction *instrPtr = arrayListGetPtr(
        DialogueInstruction,
        &(dialoguePtr->instructionList),
        *indexPtr
    );
    ++(*indexPtr);

    switch(instrPtr->command){
        case dialogue_error:
            if(instrPtr->data._ptr){
                pgWarning(instrPtr->data._ptr);
            }
            pgError("dialogue error");
            return false;
        case dialogue_setLeftImage:
            setImage(
                gamePtr,
                scenePtr,
                scenePtr->messages.dialogueData
                    .leftImageHandle,
                &(instrPtr->data)
            );
            return true;
        case dialogue_setRightImage:
            setImage(
                gamePtr,
                scenePtr,
                scenePtr->messages.dialogueData
                    .rightImageHandle,
                &(instrPtr->data)
            );
            return true;
        case dialogue_setText:
            setText(
                scenePtr,
                scenePtr->messages.dialogueData
                    .textHandle,
                &(instrPtr->data)
            );
            return true;
        case dialogue_setTrack:
            stringCopyInto(
                &(gamePtr->messages.startMusicString),
                &(instrPtr->data)
            );
            return true;
        case dialogue_stop:
            return false;
        case dialogue_end:
            gamePtr->messages.sceneExitToID
                = scene_game;
            gamePtr->messages.endDialogueFlag = true;
            return false;
        default:
            pgError(
                "unexpected default dialogue command; "
                SRC_LOCATION
            );
            return false;
    }
}

/*
 * Begins displaying the dialogue requested in game
 * messages, error if no such request is present
 */
static void initDialogue(
    Game *gamePtr,
    Scene *scenePtr
){
    /* bail if dialogue already initialized */
    if(scenePtr->messages.dialogueData.dialoguePtr){
        return;
    }

    Dialogue *dialoguePtr = NULL;
    if(stringIsEmpty(
        &(gamePtr->messages.startDialogueString)
    )){
        pgError(
            "start dialogue string empty in "
            "dialogue system; " SRC_LOCATION
        );
        return;
    }
    dialoguePtr = resourcesGetDialogue(
        gamePtr->resourcesPtr,
        &(gamePtr->messages.startDialogueString)
    );
    if(!dialoguePtr){
        pgWarning(
            gamePtr->messages.startDialogueString
                ._ptr
        );
        pgError(
            "failed to get dialogue; "
            SRC_LOCATION
        );
    }
    scenePtr->messages.dialogueData.dialoguePtr
        = dialoguePtr;

    stringClear(
        &(gamePtr->messages.startDialogueString)
    );

    while(handleNextCommand(gamePtr, scenePtr));
}

/* Handles updating dialogue graphics */
void dialogueSystem(Game *gamePtr, Scene *scenePtr){
    /* bail if not in dialogue scene */
    if(scenePtr->id != scene_dialogue){
        return;
    }

    if(!scenePtr->messages.dialogueData.dialoguePtr){
        initDialogue(gamePtr, scenePtr);
    }

    /* bail if dialogue not flagged */
    if(!scenePtr->messages.readDialogueFlag){
        return;
    }

    /* advance dialogue */
    while(handleNextCommand(gamePtr, scenePtr));
}