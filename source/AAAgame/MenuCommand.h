#ifndef MENUCOMMAND_H
#define MENUCOMMAND_H

typedef enum MenuCommand{
    menu_none,

    menu_navUp,
    menu_navDown,
    menu_navLeft,
    menu_navRight,

    menu_navFarUp,
    menu_navFarDown,
    menu_navFarLeft,
    menu_navFarRight,

    menu_enter,
    menu_enterStopMusic,

    menu_backTo,
    menu_backWriteSettings,
    menu_backSetMenuTrack,

    menu_startTrack,

    menu_toggleSound,
    menu_toggleFullscreen,

    menu_restartGame,

    menu_gameOver,

    menu_exit,

    menu_invalid,
} MenuCommand;

#endif