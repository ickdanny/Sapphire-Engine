#include "SystemCommon.h"

/*
 * Returns true if the given point is outside the
 * bounds specified relative to the game field, false
 * otherwise
 */
bool isOutOfBounds(Point2D point, float bound){
    float lowXBound = bound + config_gameOffsetX;
	float lowYBound = bound + config_gameOffsetX;
	float highXBound = config_gameWidth - bound
        + config_gameOffsetX;
	float highYBound = config_gameHeight - bound
        + config_gameOffsetY;
	return point.x < lowXBound
		|| point.x > highXBound
		|| point.y < lowYBound
		|| point.y > highYBound;
}