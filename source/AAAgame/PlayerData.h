#ifndef PLAYERDATA_H
#define PLAYERDATA_H

/* states used for the player state machine */
typedef enum PlayerState{
    player_none,
    player_normal,
    player_bombing,
    player_dead,
    player_respawning,
    player_respawnIFrames,
    player_gameOver,
} PlayerState;

/*
 * stores player lives, bombs, continues, power, and
 * state
 */
typedef struct PlayerData{
    int lives;
    int bombs;
    int continues;
    int power;
    struct {
        PlayerState playerState;
        int timer;
    } stateMachine;
} PlayerData;

#endif