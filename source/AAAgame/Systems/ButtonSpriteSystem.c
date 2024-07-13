#include "ButtonSpriteSystem.h"

/*
 * Updates the sprite and the position of the
 * specified button
 */
void changeSpriteAndPosition(
    Game *gamePtr,
    Scene *scenePtr,
    WindEntity button,
    const char *spriteName,
    Point2D position
){
    WindWorld *worldPtr = &(scenePtr->ecsWorld);

    SpriteInstruction *spriteInstructionPtr
        = windWorldHandleGetPtr(SpriteInstruction,
            worldPtr,
            button
        );
    String spriteString = stringMakeC(spriteName);
    spriteInstructionPtr->spritePtr
        = resourcesGetSprite(
            gamePtr->resourcesPtr,
            &spriteString
        );
    stringFree(&spriteString);
    
    Position *positionPtr
        = windWorldHandleGetPtr(Position,
            worldPtr,
            button
        );
    positionPtr->currentPos = position;
}

/*
 * Graphically indicates that the specified button is
 * selected
 */
void selectButton(
    Game *gamePtr,
    Scene *scenePtr,
    WindEntity button
){
    ButtonData *buttonDataPtr
        = windWorldHandleGetPtr(ButtonData,
            &(scenePtr->ecsWorld),
            button
        );
    changeSpriteAndPosition(
        gamePtr,
        scenePtr,
        button,
        buttonDataPtr->selSpriteName,
        buttonDataPtr->selPos
    );
}

/*
 * Graphically indicates that the specified button is
 * no longer selected
 */
void unselectButton(
    Game *gamePtr,
    Scene *scenePtr,
    WindEntity button
){
    ButtonData *buttonDataPtr
        = windWorldHandleGetPtr(ButtonData,
            &(scenePtr->ecsWorld),
            button
        );
    changeSpriteAndPosition(
        gamePtr,
        scenePtr,
        button,
        buttonDataPtr->unselSpriteName,
        buttonDataPtr->unselPos
    );
}

/*
 * Returns true if the specified entity has all the
 * components necessary to qualify as a button,
 * false otherwise
 */
bool isButton(Scene *scenePtr, WindEntity handle){
    WindWorld *worldPtr = &(scenePtr->ecsWorld);

    return windWorldHandleContainsComponent(
        ButtonData,
        worldPtr,
        handle
    ) && windWorldHandleContainsComponent(
        SpriteInstruction,
        worldPtr,
        handle
    ) && windWorldHandleContainsComponent(
        Position,
        worldPtr,
        handle
    );
}

/* Updates button sprites based on selection */
void buttonSpriteSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    if(scenePtr->messages.elementChanges
        .newElementSelected
    ){
        WindEntity unselectedElement = scenePtr
            ->messages.elementChanges.prevElement;
        WindEntity selectedElement = scenePtr
            ->messages.currentElement;
        if(isButton(scenePtr, unselectedElement)){
            unselectButton(
                gamePtr,
                scenePtr,
                unselectedElement
            );
        }
        if(isButton(scenePtr, selectedElement)){
            selectButton(
                gamePtr,
                scenePtr,
                selectedElement
            );
        }
    }
}