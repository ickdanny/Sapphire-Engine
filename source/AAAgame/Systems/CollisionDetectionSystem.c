#include "CollisionDetectionSystem.h"

/*
 * maximum number of elements in a quadtree before
 * a split inclusive
 */
#define quadTreeMaxElements 10

/*
 * maximum level of a quad tree, starting from 0
 * inclusive
 */
#define quadTreeMaxLevel 4

typedef struct QuadTreeElement{
    VecsEntity handle;
    AABB twoFrameHitbox;
    AABB trueHitbox;
    Position position;
} QuadTreeElement;

/* used for determining number of subframe checks */
static float _quadTreeElementSpeedRatio(
    const QuadTreeElement *elementPtr
){
    float twoFrameArea = aabbArea(
        &(elementPtr->twoFrameHitbox)
    );
    float trueArea = aabbArea(
        &(elementPtr->trueHitbox)
    );
    if(trueArea == 0.0f){
        pgError(
            "bad hitbox with 0 area; "
            SRC_LOCATION
        );
    }
    return twoFrameArea / trueArea;
}

/*
 * Returns true if the two specified quad tree elements
 * require subframe checking and collide on a subframe,
 * false if either condition is not met
 */
static bool _quadTreeElementSubframeCollides(
    const QuadTreeElement *elementPtr1,
    const QuadTreeElement *elementPtr2
){
    float speedRatio1 = _quadTreeElementSpeedRatio(
        elementPtr1
    );
    float speedRatio2 = _quadTreeElementSpeedRatio(
        elementPtr2
    );
    float largerSpeedRatio = maxFloat(
        speedRatio1,
        speedRatio2
    );

    /*
     * if neither element moves fast enough, do no
     * subframe collision checking and return false
     */
    if(largerSpeedRatio < 2.0f){
        return false;
    }

    Vector2D cartesianVelocity1 = vector2DFromAToB(
        elementPtr1->position.pastPos,
        elementPtr1->position.currentPos
    );
    Vector2D cartesianVelocity2 = vector2DFromAToB(
        elementPtr2->position.pastPos,
        elementPtr2->position.currentPos
    );

    int numSubframes = ((int)largerSpeedRatio) - 1;
    float baseDelta = 1.0f / ((float)numSubframes + 1);
    for(int i = 1; i <= numSubframes; ++i){
        /* float from 0 to 1 for interpolation */
        float delta = baseDelta * i;
        Point2D interpolatedPosition1
            = point2DAddVector2D(
                elementPtr1->position.pastPos,
                vector2DMultiply(
                    cartesianVelocity1,
                    delta
                )
            );
        Point2D interpolatedPosition2
            = point2DAddVector2D(
                elementPtr2->position.pastPos,
                vector2DMultiply(
                    cartesianVelocity2,
                    delta
                )
            );
        AABB interpolatedHitbox1 = aabbCenterAt(
            &(elementPtr1->trueHitbox),
            interpolatedPosition1
        );
        AABB interpolatedHitbox2 = aabbCenterAt(
            &(elementPtr2->trueHitbox),
            interpolatedPosition2
        );
        if(aabbCollides(
            &(interpolatedHitbox1),
            &(interpolatedHitbox2)
        )){
            return true;
        }
    }
    return false;
}

/*
 * Returns true if the two given quadtree elements
 * collide, false otherwise
 */
static bool _quadTreeElementCollides(
    const QuadTreeElement *elementPtr1,
    const QuadTreeElement *elementPtr2
){
    /*
     * if the two frame hitboxes miss, clearly no
     * collision
     */
    if(!aabbCollides(
        &(elementPtr1->twoFrameHitbox),
        &(elementPtr2->twoFrameHitbox)
    )){
        return false;
    }

    /* if true hitboxes hit, collision */
    if(aabbCollides(
        &(elementPtr1->trueHitbox),
        &(elementPtr2->trueHitbox)
    )){
        return true;
    }

    /* otherwise, do subframe checking */
    return _quadTreeElementSubframeCollides(
        elementPtr1,
        elementPtr2
    );
}

/* spatial partition for collision detection */
typedef struct QuadTree{
    /* root node is 0, growing upwards */
    int level;

    /*
     * the bounds of the quadtree which all of its
     * elements are to be within
     */
    AABB bounds;

    /* array of four children */
    struct QuadTree *children[4];

    /* list of QuadTreeElement */
    ArrayList elementList;
} QuadTree;

/* Creates and returns a new QuadTree by value */
static QuadTree quadTreeMake(int level, AABB bounds){
    QuadTree toRet = {0};
    toRet.level = level;
    toRet.bounds = bounds;
    toRet.elementList = arrayListMake(QuadTreeElement,
        quadTreeMaxElements
    );

    return toRet;
}

/*
 * Returns true if the specified quadtree has children,
 * false otherwise
 */
#define _quadTreeHasChildren(TREEPTR) \
    (TREEPTR->children[0])

/*
 * Returns true if the given hitbox collides with the
 * bounds of the specified quadtree, false otherwise
 */
static bool _quadTreeCollides(
    const QuadTree *quadTreePtr,
    const AABB *hitboxPtr
){
    return aabbCollides(
        &(quadTreePtr->bounds),
        hitboxPtr
    );
}

/*
 * Returns true if the specified quadtree can split,
 * false otherwise; a tree cannot split if it already
 * has children or its level is too high
 */
#define _quadTreeCanSplit(TREEPTR) \
    (!_quadTreeHasChildren(TREEPTR) \
        && TREEPTR->level < quadTreeMaxLevel)

/*
 * Recursively populates a collision list for a
 * specified quadtree element
 */
static void _quadTreePopulateCollisionList(
    QuadTree *quadTreePtr,
    const QuadTreeElement *elementPtr,
    ArrayList *collisionListPtr /* type VecsEntity */
){
    /* check collisions in the root quad tree */
    for(size_t i = 0;
        i < quadTreePtr->elementList.size;
        ++i
    ){
        QuadTreeElement *toCheckPtr = arrayListGetPtr(
            QuadTreeElement,
            &(quadTreePtr->elementList),
            i
        );
        if(_quadTreeElementCollides(
            elementPtr,
            toCheckPtr
        )){
            arrayListPushBack(VecsEntity,
                collisionListPtr,
                toCheckPtr->handle
            );
        }
    }

    /* check collisions in subtrees */
    if(_quadTreeHasChildren(quadTreePtr)){
        for(int i = 0; i < 4; ++i){
            QuadTree *childPtr
                = quadTreePtr->children[i];
            if(_quadTreeCollides(
                childPtr,
                &(elementPtr->twoFrameHitbox)
            )){
                _quadTreePopulateCollisionList(
                    childPtr,
                    elementPtr,
                    collisionListPtr
                );
            }
        }
    }
}

/* forward declare */
static void _quadTreeInsertElement(
    QuadTree *quadTreePtr,
    QuadTreeElement *elementPtr
);

/*
 * Distributes the elements held by the specified
 * quadtree among its children
 */
static void _quadTreeSplitElements(
    QuadTree *quadTreePtr
){
    /* make a shallow copy of old element list */
    ArrayList oldElements = quadTreePtr->elementList;

    /* make new element list */
    quadTreePtr->elementList = arrayListMake(
        QuadTreeElement,
        quadTreeMaxElements
    );

    /* insert every old element */
    for(size_t i = 0; i < oldElements.size; ++i){
        _quadTreeInsertElement(
            quadTreePtr,
            arrayListGetPtr(QuadTreeElement,
                &oldElements,
                i
            )
        );
    }

    /* free the old element list */
    arrayListFree(QuadTreeElement, &oldElements);
}

/* Splits the specified quadtree if necessary */
static void _quadTreeSplit(QuadTree *quadTreePtr){
    /* bail if cannot split */
    if(!_quadTreeCanSplit(quadTreePtr)){
        return;
    }

    /*
     * if not more than max elements, no need to split
     */
    if(quadTreePtr->elementList.size
        <= quadTreeMaxElements
    ){
        return;
    }

    /* calculate new bounds and create children */
    float xLow = quadTreePtr->bounds.xLow;
    float xHigh = quadTreePtr->bounds.xHigh;
    float yLow = quadTreePtr->bounds.yLow;
    float yHigh = quadTreePtr->bounds.yHigh;

    float xAvg = (xLow + xHigh) / 2.0f;
    float yAvg = (yLow = yHigh) / 2.0f;

    int childLevel = quadTreePtr->level + 1;

    quadTreePtr->children[0] = pgAlloc(
        1,
        sizeof(*(quadTreePtr->children[0]))
    );
    quadTreePtr->children[1] = pgAlloc(
        1,
        sizeof(*(quadTreePtr->children[1]))
    );
    quadTreePtr->children[2] = pgAlloc(
        1,
        sizeof(*(quadTreePtr->children[2]))
    );
    quadTreePtr->children[3] = pgAlloc(
        1,
        sizeof(*(quadTreePtr->children[3]))
    );
    *quadTreePtr->children[0] = quadTreeMake(
        childLevel,
        (AABB){xLow, xAvg, yLow, yAvg}
    );
    *quadTreePtr->children[1] = quadTreeMake(
        childLevel,
        (AABB){xAvg, xHigh, yLow, yAvg}
    );
    *quadTreePtr->children[2] = quadTreeMake(
        childLevel,
        (AABB){xAvg, xHigh, yAvg, yHigh}
    );
    *quadTreePtr->children[3] = quadTreeMake(
        childLevel,
        (AABB){xLow, xAvg, yAvg, yHigh}
    );

    /* populate children */
    _quadTreeSplitElements(quadTreePtr);
}

/*
 * Adds the given element to the specified quadtree and
 * splits if necessary
 */
static void _quadTreeAddElement(
    QuadTree *quadTreePtr,
    QuadTreeElement *elementPtr
){
    arrayListPushBackPtr(QuadTreeElement,
        &(quadTreePtr->elementList),
        elementPtr
    );
    _quadTreeSplit(quadTreePtr);
}

/*
 * Inserts the given element into the specified
 * quadtree or one of its children if the element
 * is within its bounds
 */
static void _quadTreeInsertElement(
    QuadTree *quadTreePtr,
    QuadTreeElement *elementPtr
){
    /*
     * bail if the element doesn't fit in the
     * quadtree
     */
    if(!_quadTreeCollides(
        quadTreePtr,
        &(elementPtr->twoFrameHitbox)
    )){
        return;
    }

    if(_quadTreeHasChildren(quadTreePtr)){
        /*
         * check how many children collide with the
         * element
         */
        QuadTree *collidedChildPtr = NULL;
        int numChildrenCollided = 0;
        for(int i = 0; i < 4; ++i){
            QuadTree *childPtr
                = quadTreePtr->children[i];
            if(_quadTreeCollides(
                childPtr,
                &(elementPtr->twoFrameHitbox)
            )){
                collidedChildPtr = childPtr;
                ++numChildrenCollided;
            }
        }

        /*
         * if only one child collided, insert into that
         * one
         */
        if(numChildrenCollided == 1){
            _quadTreeInsertElement(
                collidedChildPtr,
                elementPtr
            );
        }
        /* otherwise, insert into top level (this) */
        _quadTreeAddElement(quadTreePtr, elementPtr);
    }
    else{
        _quadTreeAddElement(quadTreePtr, elementPtr);
    }
}

/*
 * Returns true if the specified quad tree has no
 * elements, false otherwise
 */
static bool quadTreeIsEmpty(QuadTree *quadTreePtr){
    if(_quadTreeHasChildren(quadTreePtr)){
        for(int i = 0; i < 4; ++i){
            if(!quadTreeIsEmpty(
                quadTreePtr->children[i]
            )){
                return false;
            }
        }
    }

    return arrayListIsEmpty(
        &(quadTreePtr->elementList)
    );
}

/*
 * Populates the specified VecsEntity list with the
 * handles of all the entities in the quadtree that
 * collide with the object specified by the given
 * hitbox and position
 */
static void quadTreePopulateCollisionList(
    QuadTree *quadTreePtr,
    ArrayList *collisionListPtr,
    const AABB *hitboxPtr,
    const Position *positionPtr
){
    /* create a dummy quad tree element */
    QuadTreeElement dummyElement = {0};
    dummyElement.position = *positionPtr;
    dummyElement.trueHitbox = aabbCenterAt(
        hitboxPtr,
        dummyElement.position.currentPos
    );
    AABB pastHitbox = aabbCenterAt(
        hitboxPtr,
        dummyElement.position.pastPos
    );
    dummyElement.twoFrameHitbox = aabbMakeEncompassing(
        &(dummyElement.trueHitbox),
        &pastHitbox
    );
    _quadTreePopulateCollisionList(
        quadTreePtr,
        &dummyElement,
        collisionListPtr
    );
}

/*
 * Inserts the given object into the specified quadtree
 * if it falls within the bounds of the quadtree
 */
static void quadTreeInsert(
    QuadTree *quadTreePtr,
    VecsEntity handle,
    AABB *hitboxPtr,
    Position *positionPtr
){
    /* create element for the object*/
    QuadTreeElement element = {0};
    element.handle = handle;
    element.position = *positionPtr;
    element.trueHitbox = aabbCenterAt(
        hitboxPtr,
        element.position.currentPos
    );
    AABB pastHitbox = aabbCenterAt(
        hitboxPtr,
        element.position.pastPos
    );
    element.twoFrameHitbox = aabbMakeEncompassing(
        &(element.trueHitbox),
        &pastHitbox
    );

    /* if element falls within bounds, insert it */
    if(_quadTreeCollides(
        quadTreePtr,
        &(element.twoFrameHitbox)
    )){
        _quadTreeInsertElement(quadTreePtr, &element);
    }
}

/* Frees the memory associated with a given QuadTree */
static void quadTreeFree(QuadTree *quadTreePtr){
    if(!quadTreePtr){
        return;
    }

    quadTreeFree(quadTreePtr->children[0]);
    quadTreeFree(quadTreePtr->children[1]);
    quadTreeFree(quadTreePtr->children[2]);
    quadTreeFree(quadTreePtr->children[3]);
    pgFree(quadTreePtr->children[0]);
    pgFree(quadTreePtr->children[1]);
    pgFree(quadTreePtr->children[2]);
    pgFree(quadTreePtr->children[3]);

    arrayListFree(QuadTreeElement,
        &(quadTreePtr->elementList)
    );

    memset(quadTreePtr, 0, sizeof(*quadTreePtr));
}

/*
 * declares a set of functions for checking collisions
 * of a specific type
 */
#define COLLISION_TYPE_DECLARE(PREFIX, SUFFIX) \
static VecsComponentSet sourceSet##SUFFIX \
    = vecsComponentSetFromId(VecsEntityId) \
    | vecsComponentSetFromId(PositionId) \
    | vecsComponentSetFromId(HitboxId) \
    | vecsComponentSetFromId(CollidableMarkerId) \
    | vecsComponentSetFromId( \
        SUFFIX##CollisionSourceId \
    ); \
static VecsComponentSet targetSet##SUFFIX \
    = vecsComponentSetFromId(VecsEntityId) \
    | vecsComponentSetFromId(PositionId) \
    | vecsComponentSetFromId(HitboxId) \
    | vecsComponentSetFromId(CollidableMarkerId) \
    | vecsComponentSetFromId( \
        SUFFIX##CollisionTargetId \
    ); \
\
/* checks for collisions of a specific type */ \
void detectCollisions##SUFFIX(Scene *scenePtr){ \
    /* clear the collision channel */ \
    ArrayList *collisionChannelPtr \
        = &(scenePtr->messages \
            .PREFIX##CollisionList); \
    arrayListClear(Collision, \
        collisionChannelPtr \
    ); \
    \
    /* get source entities and put in quadtree */ \
    QuadTree quadTree = quadTreeMake( \
        0, \
        config_collisionBounds \
    ); \
    VecsQueryItr sourceItr \
        = vecsWorldRequestQueryItr( \
            &(scenePtr->ecsWorld), \
            sourceSet##SUFFIX, \
            vecsEmptyComponentSet \
        ); \
    while(vecsQueryItrHasEntity(&sourceItr)){ \
        Position *positionPtr = vecsQueryItrGetPtr( \
            Position, \
            &sourceItr \
        ); \
        Hitbox *hitboxPtr = vecsQueryItrGetPtr( \
            Hitbox, \
            &sourceItr \
        ); \
        VecsEntity entity = vecsQueryItrGet( \
            VecsEntity, \
            &sourceItr \
        ); \
        quadTreeInsert( \
            &quadTree, \
            entity, \
            hitboxPtr, \
            positionPtr \
        ); \
        vecsQueryItrAdvance(&sourceItr); \
    } \
    \
    /* bail if quad tree is empty */ \
    if(quadTreeIsEmpty(&quadTree)){ \
        quadTreeFree(&quadTree); \
        return; \
    } \
    \
    /* check all targets against the quad tree */ \
    ArrayList collisionList = arrayListMake( \
        VecsEntity, \
        10 \
    ); \
    VecsQueryItr targetItr \
        = vecsWorldRequestQueryItr( \
            &(scenePtr->ecsWorld), \
            targetSet##SUFFIX, \
            vecsEmptyComponentSet \
        ); \
    while(vecsQueryItrHasEntity(&targetItr)){ \
        Position *positionPtr = vecsQueryItrGetPtr( \
            Position, \
            &targetItr \
        ); \
        Hitbox *hitboxPtr = vecsQueryItrGetPtr( \
            Hitbox, \
            &targetItr \
        ); \
        quadTreePopulateCollisionList( \
            &quadTree, \
            &collisionList, \
            hitboxPtr, \
            positionPtr \
        ); \
        if(!arrayListIsEmpty(&collisionList)){ \
            VecsEntity target = vecsQueryItrGet( \
                VecsEntity, \
                &targetItr \
            ); \
            for(size_t i = 0; \
                i < collisionList.size; \
                ++i \
            ){ \
                VecsEntity source = arrayListGet( \
                    VecsEntity, \
                    &collisionList, \
                    i \
                ); \
                if(target != source){ \
                    arrayListPushBack(Collision, \
                        collisionChannelPtr, \
                        ((Collision){ \
                            source, \
                            target \
                        }) \
                    ); \
                } \
            } \
        } \
        \
        arrayListClear(VecsEntity, &collisionList); \
        vecsQueryItrAdvance(&targetItr); \
    } \
    arrayListFree(VecsEntity, &collisionList); \
    quadTreeFree(&quadTree); \
}

COLLISION_TYPE_DECLARE(player, Player)
COLLISION_TYPE_DECLARE(enemy, Enemy)
COLLISION_TYPE_DECLARE(bullet, Bullet)
COLLISION_TYPE_DECLARE(pickup, Pickup)

#undef COLLISION_TYPE_DECLARE

/*
 * Detects collisions between sources and targets of
 * the same collision type
 */
void collisionDetectionSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    detectCollisionsPlayer(scenePtr);
    detectCollisionsEnemy(scenePtr);
    detectCollisionsBullet(scenePtr);
    detectCollisionsPickup(scenePtr);
}