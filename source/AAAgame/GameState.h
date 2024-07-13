#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdint.h>

/*
 * Story mode means going through each stage in
 * sequence, practice mode means only playing a single
 * stage at a time
 */
typedef enum GameMode{
    game_story,
    game_practice
} GameMode;

/*
 * The difficulty of the game influences the amount of
 * bullets the player must dodge
 */
typedef enum Difficulty{
    game_normal,
    game_hard,
    game_lunatic
} Difficulty;

/*
 * Stores information about the game being played
 * including whether it is practice mode and the
 * difficulty of the game
 */
typedef struct GameState{
    GameMode gameMode;
    Difficulty difficulty;
    int stage;
    uint32_t prngSeed;
} GameState;

#endif