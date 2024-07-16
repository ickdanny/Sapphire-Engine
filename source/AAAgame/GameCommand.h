#ifndef GAMECOMMAND_H
#define GAMECOMMAND_H

/* commands sent to the game scene */
typedef enum GameCommand{
    game_none,
    game_pause,
    game_focus,
    game_shoot,
    game_bomb,
    game_up,
    game_down,
    game_left,
    game_right,
} GameCommand;

#endif