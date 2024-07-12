#ifndef GAMEBUILDERCOMMAND_H
#define GAMEBUILDERCOMMAND_H

typedef enum GameBuilderCommand{
    gb_none,
    gb_start,
    gb_practice,

    gb_normal,
    gb_hard,
    gb_lunatic,

    gb_stage1,
    gb_stage2,
    gb_stage3,
    gb_stage4,

    gb_reset,

    gb_invalid,
} GameBuilderCommand;

#endif