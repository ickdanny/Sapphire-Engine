#ifndef COLLISIONCOMMAND_H
#define COLLISIONCOMMAND_H

/*
 * Defines the behavior an entity exhibits when
 * colliding with another entity
 */
typedef enum CollisionCommand{
    collision_none,
    /* die upon collision */
    collision_death,
    /* take damage upon collision */
    collision_damage,
    /* remove the collision type upon collision */
    collision_removeType,
    /* special collision type for the player */
    collision_player,
    /* special collision type for pickups */
    collision_pickup
} CollisionCommand;

#endif