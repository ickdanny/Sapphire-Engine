#include "ButtonSpriteSystem.h"

/*
 * Updates the sprite and the position of the
 * specified button
 */
void changeSpriteAndPosition(
    Game *gamePtr,
    Scene *scenePtr,
    VecsEntity button,
    const char *spriteName,
    Point2D position
){
    VecsWorld *worldPtr = &(scenePtr->ecsWorld);

    SpriteInstruction *spriteInstructionPtr
        = vecsWorldEntityGetPtr(SpriteInstruction,
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
        = vecsWorldEntityGetPtr(Position,
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
    VecsEntity button
){
    ButtonData *buttonDataPtr
        = vecsWorldEntityGetPtr(ButtonData,
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
    VecsEntity button
){
    ButtonData *buttonDataPtr
        = vecsWorldEntityGetPtr(ButtonData,
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
bool isButton(Scene *scenePtr, VecsEntity handle){
    VecsWorld *worldPtr = &(scenePtr->ecsWorld);

    return vecsWorldEntityContainsComponent(
        ButtonData,
        worldPtr,
        handle
    ) && vecsWorldEntityContainsComponent(
        SpriteInstruction,
        worldPtr,
        handle
    ) && vecsWorldEntityContainsComponent(
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
        VecsEntity unselectedElement = scenePtr
            ->messages.elementChanges.prevElement;
        VecsEntity selectedElement = scenePtr
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