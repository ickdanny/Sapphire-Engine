#ifndef SYSTEMCOMMON_H
#define SYSTEMCOMMON_H

#include "Game.h"
#include "Components.h"
#include "EntityBuilder.h"
#include "SystemDestructors.h"
#include "Config.h"

/* some functions which are used by multiple systems */

/*
 * Returns true if the given point is outside the
 * bounds specified relative to the game field, false
 * otherwise
 */
bool isOutOfBounds(Point2D point, float bound);

#endif