#include "TextRenderSystem.h"

#define horizontalDist 7
#define verticalDist 11

static Bitset accept;
static TFGlyphMap glyphMap;
static bool initialized = false;

/* destroys the text render system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        TFGlyphMapFree(&glyphMap);
        initialized = false;
    }
}

/* inits the text render system */
static void init(Game *gamePtr){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, PositionID);
        bitsetSet(&accept, VisibleMarkerID);
        bitsetSet(&accept, TextInstructionID);

        glyphMap = tfGlyphMapMake(
            horizontalDist,
            verticalDist
        );

        String spriteNameString
            = stringMakeAndReserve(20);
        TFSpriteInstruction spriteInstr = {0};
        
        #define insertGlyph(WCHAR, SPRITEID) \
            stringClear(&spriteNameString); \
            stringAppendC( \
                &spriteNameString, \
                #SPRITEID \
            ); \
            spriteInstr \
                = tfSpriteInstructionMakeSimple( \
                    resourcesGetSprite( \
                        gamePtr->resourcesPtr, \
                        &spriteNameString \
                    ), \
                    config_foregroundDepth, \
                    ((Vector2D){0}) \
                ); \
            tfGlyphMapPut( \
                &glyphMap, \
                WCHAR, \
                &spriteInstr \
            );

        /* lowercase letters */
        insertGlyph('a', symbol_a);
        insertGlyph('b', symbol_b);
        insertGlyph('c', symbol_c);
        insertGlyph('d', symbol_d);
        insertGlyph('e', symbol_e);
        insertGlyph('f', symbol_f);
        insertGlyph('g', symbol_g);
        insertGlyph('h', symbol_h);
        insertGlyph('i', symbol_i);
        insertGlyph('j', symbol_j);
        insertGlyph('k', symbol_k);
        insertGlyph('l', symbol_l);
        insertGlyph('m', symbol_m);
        insertGlyph('n', symbol_n);
        insertGlyph('o', symbol_o);
        insertGlyph('p', symbol_p);
        insertGlyph('q', symbol_q);
        insertGlyph('r', symbol_r);
        insertGlyph('s', symbol_s);
        insertGlyph('t', symbol_t);
        insertGlyph('u', symbol_u);
        insertGlyph('v', symbol_v);
        insertGlyph('w', symbol_w);
        insertGlyph('x', symbol_x);
        insertGlyph('y', symbol_y);
        insertGlyph('z', symbol_z);

        /* uppercase letters */
        insertGlyph('A', symbol__A);
        insertGlyph('B', symbol__B);
        insertGlyph('C', symbol__C);
        insertGlyph('D', symbol__D);
        insertGlyph('E', symbol__E);
        insertGlyph('F', symbol__F);
        insertGlyph('G', symbol__G);
        insertGlyph('H', symbol__H);
        insertGlyph('I', symbol__I);
        insertGlyph('J', symbol__J);
        insertGlyph('K', symbol__K);
        insertGlyph('L', symbol__L);
        insertGlyph('M', symbol__M);
        insertGlyph('N', symbol__N);
        insertGlyph('O', symbol__O);
        insertGlyph('P', symbol__P);
        insertGlyph('Q', symbol__Q);
        insertGlyph('R', symbol__R);
        insertGlyph('S', symbol__S);
        insertGlyph('T', symbol__T);
        insertGlyph('U', symbol__U);
        insertGlyph('V', symbol__V);
        insertGlyph('W', symbol__W);
        insertGlyph('X', symbol__X);
        insertGlyph('Y', symbol__Y);
        insertGlyph('Z', symbol__Z);

        /* numerals */
        insertGlyph('0', symbol_0);
        insertGlyph('1', symbol_1);
        insertGlyph('2', symbol_2);
        insertGlyph('3', symbol_3);
        insertGlyph('4', symbol_4);
        insertGlyph('5', symbol_5);
        insertGlyph('6', symbol_6);
        insertGlyph('7', symbol_7);
        insertGlyph('8', symbol_8);
        insertGlyph('9', symbol_9);

        /* special */
        insertGlyph('*', symbol_asterisk);
        insertGlyph(':', symbol_colon);
        insertGlyph(',', symbol_comma);
        insertGlyph('"', symbol_dquote);
        insertGlyph('!', symbol_exclaim);
        insertGlyph('.', symbol_period);
        insertGlyph('?', symbol_question);
        insertGlyph('\'', symbol_quote);
        insertGlyph(';', symbol_semicolon);
        insertGlyph('~', symbol_tilde);

        #undef insertGlyph

        stringFree(&spriteNameString);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* renders text to screen */
void textRenderSystem(Game *gamePtr, Scene *scenePtr){
    init(gamePtr);
    
    /* get entities with position and text instr */
    WindQueryItr itr = windWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    while(windQueryItrHasEntity(&itr)){
        Position *positionPtr = windQueryItrGetPtr(
            Position,
            &itr
        );
        TextInstruction *textInstrPtr
            = windQueryItrGetPtr(TextInstruction,
                &itr
            );
        tfWindowDrawText(
            gamePtr->windowPtr,
            positionPtr->currentPos,
            &(textInstrPtr->text),
            textInstrPtr->rightBound,
            &glyphMap
        );
        windQueryItrAdvance(&itr);
    }
}