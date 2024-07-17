#ifndef COLLISIONTYPE_H
#define COLLISIONTYPE_H

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

typedef enum CollisionSourceOrTarget{
    collision_source,
    collision_target
} CollisionSourceOrTarget;

/*
 * Represents the behavior an entity should exhibit
 * upon collision with another entity
 */
typedef struct CollisionType{
    CollisionSourceOrTarget type;
    CollisionCommand command;
} CollisionType;

#endif