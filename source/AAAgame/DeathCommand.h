#ifndef DEATHCOMMAND_H
#define DEATHCOMMAND_H

/*
 * Represents the different behaviors entities can
 * exhibit upon being flagged as dead
 */
typedef enum DeathCommand{
    /* default behavior: remove entity on death */
    death_remove,
    /* special command for player logic */
    death_player,
    /* special command for boss logic */
    death_boss,
    /*
     * indicates the presence of a death script
     * component which should be used to spawn a ghost
     */
    death_script,
} DeathCommand;

#endif