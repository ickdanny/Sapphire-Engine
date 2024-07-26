#include "Prototypes.h"

/* PLAYER PROTOTYPE CONFIG */
#define shardHitboxRadius 9.0f
#define shardDamage 100
#define shardOutbound -30.0f

#define caltropHitboxRadius 8.0f
#define caltropDamage 400
#define caltropOutbound -30.0f /* just in case */

#define spikeHitboxRadius 5.0f
#define spikeDamage 100
#define spikeOutbound -30.0f

#define bombHitboxRadius 20.0f
#define bombDamagePerTick 1 /* small; bomb explodes */
#define bombOutbound -100.0f /* just in case */

#define bombExplodeHitboxRadius 27.0f
#define bombExplodeDamage 2000

/* PICKUP PROTOTYPE CONFIG */
#define smallPickupHitboxRadius 8.0f
#define largePickupHitboxRadius 12.0f
#define pickupOutbound -100.0f
#define smallPowerGain 1
#define largePowerGain 5

/* ENEMY PROTOTYPE CONFIG */
/* set health in each script */
#define enemySpawnHealth 9999999
#define enemyOutbound -30.0f
#define mediumStarHitboxRadius 7.0f
#define largeStarHitboxRadius 10.0f
#define birdHitboxRadius 6.0f
#define machineHitboxRadius 9.0f
#define fireflyHitboxRadius 8.5f
#define beeHitboxRadius 8.5f
#define plantHitbox (aabbMakeXY(8.0f, 10.0f))
#define blobHitboxRadius 11.0f
#define automatonHitbox (aabbMakeXY(3.0f, 7.0f))
#define wispHitboxRadius 7.5f
#define crystalHitbox (aabbMakeXY(5.0f, 9.5f))
#define bossHitbox (aabbMakeXY(7.0f, 13.0f))
#define pickupWispHitboxRadius 10.0f

#define birdAnimationMaxTick 3
#define machineAnimationMaxTick 3
#define fireflyAnimationMaxTick 2
#define beeAnimationMaxTick 2
#define plantAnimationMaxTick 5
#define blobAnimationMaxTick 5
#define automatonAnimationMaxTick 5
#define wispAnimationMaxTick 5
#define crystalAnimationMaxTick 5
#define bossAnimationMaxTick 5
#define pickupWispAnimationMaxTick 3

#define trapSpin -2.345f
#define starSpin 2.3f

/* BULLET PROTOTYPE CONFIG */
#define smallHitboxRadius 2.5f
#define mediumHitboxRadius 3.5f
#define largeHitboxRadius 7.0f
#define sharpHitboxRadius 1.5f
#define laserHitboxRadius 1.8f
#define starHitboxRadius 2.0f
#define bulletOutbound -30.0f

typedef void (*PrototypeFunction)(
    Game*,
    Scene*,
    ArrayList*,
    int
);

#define DECLARE_PROTOTYPE(NAME) \
    static void NAME( \
        Game *gamePtr, \
        Scene *scenePtr, \
        ArrayList *componentListPtr, \
        int depthOffset \
    )

/* PLAYER PROTOTYPES */

DECLARE_PROTOTYPE(shard){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbMakeRadius(shardHitboxRadius)
    );
    addEnemyCollisionSource(
        componentListPtr,
        collision_death
    );
    addDamage(componentListPtr, shardDamage);
    addOutbound(componentListPtr, shardOutbound);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "shard",
        config_playerBulletDepth + depthOffset,
        (Vector2D){0}
    );
    addRotateSpriteForward(componentListPtr);
    addDeathCommand(componentListPtr, death_script);
    addDeathScripts(componentListPtr, ((DeathScripts){
        .scriptID1 = stringMakeC("remove_ghost"),
        .scriptID3 = stringMakeC(
            "spawn_explode_projectile"
        )
    }));
}

DECLARE_PROTOTYPE(caltrop){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbMakeRadius(caltropHitboxRadius)
    );
    addEnemyCollisionSource(
        componentListPtr,
        collision_death
    );
    addDamage(componentListPtr, caltropDamage);
    addOutbound(componentListPtr, caltropOutbound);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "caltrop",
        config_playerBulletDepth + depthOffset,
        (Vector2D){0}
    );
    addDeathCommand(componentListPtr, death_script);
    addDeathScripts(componentListPtr, ((DeathScripts){
        .scriptID1 = stringMakeC("remove_ghost"),
        .scriptID3 = stringMakeC(
            "spawn_explode_projectile"
        ),
        .scriptID4 = stringMakeC("death_caltrop")
    }));
}

#define DECLARE_SPIKE_PROTOTYPE(NAME) \
    DECLARE_PROTOTYPE(NAME){ \
        addVisible(componentListPtr); \
        addCollidable(componentListPtr); \
        addHitbox( \
            componentListPtr, \
            aabbMakeRadius(spikeHitboxRadius) \
        ); \
        addEnemyCollisionSource( \
            componentListPtr, \
            collision_death \
        ); \
        addDamage(componentListPtr, spikeDamage); \
        addOutbound(componentListPtr, spikeOutbound); \
        addSpriteInstructionSimple( \
            componentListPtr, \
            gamePtr, \
            #NAME, \
            config_playerBulletDepth + depthOffset, \
            (Vector2D){0} \
        ); \
        addDeathCommand( \
            componentListPtr, \
            death_script \
        ); \
        addDeathScripts( \
            componentListPtr, \
            ((DeathScripts){ \
                .scriptID1 = stringMakeC( \
                    "remove_ghost" \
                ), \
                .scriptID3 = stringMakeC( \
                    "spawn_explode_projectile" \
                ) \
            }) \
        ); \
    }
DECLARE_SPIKE_PROTOTYPE(spike_bottomLeft)
DECLARE_SPIKE_PROTOTYPE(spike_bottomRight)
DECLARE_SPIKE_PROTOTYPE(spike_topLeft)
DECLARE_SPIKE_PROTOTYPE(spike_topRight)
#undef DECLARE_SPIKE_PROTOTYPE

DECLARE_PROTOTYPE(bomb){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbMakeRadius(bombHitboxRadius)
    );
    /* bomb explodes on enemies */
    addEnemyCollisionSource(
        componentListPtr,
        collision_death
    );
    /*
     * bullets collide with bomb but nothing happens
     * to bomb
     */
    addBulletCollisionSource(
        componentListPtr,
        collision_none
    );
    addDamage(componentListPtr, bombDamagePerTick);
    addOutbound(componentListPtr, bombOutbound);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "bomb1",
        config_playerBulletDepth + depthOffset,
        (Vector2D){0}
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(&animation, "bomb1");
    animationAddFrame(&animation, "bomb2");
    animationAddFrame(&animation, "bomb3");
    animationAddFrame(&animation, "bomb4");
    animationAddFrame(&animation, "bomb5");
    animationAddFrame(&animation, "bomb6");
    animationAddFrame(&animation, "bomb7");
    animationAddFrame(&animation, "bomb8");
    animationAddFrame(&animation, "bomb9");
    animationAddFrame(&animation, "bomb10");
    animationAddFrame(&animation, "bomb11");
    animationAddFrame(&animation, "bomb12");
    animationAddFrame(&animation, "bomb13");
    animationAddFrame(&animation, "bomb14");
    animationAddFrame(&animation, "bomb15");
    animationAddFrame(&animation, "bomb16");
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 3;
    addAnimations(componentListPtr, &animations);

    addRotateSpriteForward(componentListPtr);
    addDeathCommand(componentListPtr, death_script);
    addDeathScripts(componentListPtr, ((DeathScripts){
        .scriptID1 = stringMakeC("remove_ghost"),
        .scriptID3 = stringMakeC(
            "spawn_explode_bomb"
        )
    }));
}

DECLARE_PROTOTYPE(explode_bomb){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbMakeRadius(bombExplodeHitboxRadius)
    );
    addBulletCollisionSource(
        componentListPtr,
        collision_none
    );
    addEnemyCollisionSource(
        componentListPtr,
        collision_none
    );
    addDamage(componentListPtr, bombExplodeDamage);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "explode_bomb1",
        config_playerBulletDepth + depthOffset,
        (Vector2D){0}
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(&animation, "explode_bomb1");
    animationAddFrame(&animation, "explode_bomb2");
    animationAddFrame(&animation, "explode_bomb3");
    animationAddFrame(&animation, "explode_bomb4");
    animationAddFrame(&animation, "explode_bomb5");
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 2;
    addAnimations(componentListPtr, &animations);
}

/* PICKUP PROTOTYPES */

DECLARE_PROTOTYPE(power_small){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbMakeRadius(smallPickupHitboxRadius)
    );
    addPickupCollisionSource(
        componentListPtr,
        collision_pickup
    );
    addPowerGain(componentListPtr, smallPowerGain);
    addOutbound(componentListPtr, pickupOutbound);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "pickup_powerSmall",
        config_pickupDepth + depthOffset,
        (Vector2D){0}
    );
}

DECLARE_PROTOTYPE(power_large){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbMakeRadius(largePickupHitboxRadius)
    );
    addPickupCollisionSource(
        componentListPtr,
        collision_pickup
    );
    addPowerGain(componentListPtr, largePowerGain);
    addOutbound(componentListPtr, pickupOutbound);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "pickup_powerLarge",
        config_pickupDepth + depthOffset,
        (Vector2D){0}
    );
}

/* ENEMY PROTOTYPES */

/* adds the basic components for an enemy */
#define addEnemyBaseComponents( \
    HITBOX, \
    SPRITEID, \
    DEATHSCRIPT3ID \
) \
    do{ \
        addVisible(componentListPtr); \
        addCollidable(componentListPtr); \
        addHitbox(componentListPtr, HITBOX); \
        addPlayerCollisionSource( \
            componentListPtr, \
            collision_none \
        ); \
        addEnemyCollisionTarget( \
            componentListPtr, \
            collision_damage \
        ); \
        addDamage(componentListPtr, 1); \
        addHealth( \
            componentListPtr, \
            enemySpawnHealth \
        ); \
        addSpriteInstructionSimple( \
            componentListPtr, \
            gamePtr, \
            #SPRITEID, \
            config_enemyDepth + depthOffset, \
            (Vector2D){0} \
        ); \
        addOutbound(componentListPtr, enemyOutbound); \
        addDeathCommand( \
            componentListPtr, \
            death_script \
        ); \
        addDeathScripts( \
            componentListPtr, \
            ((DeathScripts){ \
                .scriptID1 = stringMakeC( \
                    "remove_ghost" \
                ), \
                .scriptID3 = stringMakeC( \
                    #DEATHSCRIPT3ID \
                ) \
            }) \
        ); \
    } while(false)

DECLARE_PROTOTYPE(star_medium){
    addEnemyBaseComponents(
        aabbMakeRadius(mediumStarHitboxRadius),
        star_medium,
        spawn_explode_enemy
    );
    float spin = 0;
    if(zmtRandBool(&scenePtr->messages.prng)){
        spin = starSpin;
    }
    else{
        spin = -starSpin;
    }
    addSpriteSpin(componentListPtr, spin);
}

DECLARE_PROTOTYPE(star_large){
    addEnemyBaseComponents(
        aabbMakeRadius(largeStarHitboxRadius),
        star_large,
        spawn_explode_enemy
    );
    float spin = 0;
    if(zmtRandBool(&scenePtr->messages.prng)){
        spin = starSpin;
    }
    else{
        spin = -starSpin;
    }
    addSpriteSpin(componentListPtr, spin);
}

DECLARE_PROTOTYPE(bird_blue){
    addEnemyBaseComponents(
        aabbMakeRadius(birdHitboxRadius),
        bird_blue1,
        spawn_explode_enemy
    );
    addRotateSpriteForward(componentListPtr);
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "bird_blue1"
    );
    animationAddFrame(
        &animation,
        "bird_blue2"
    );
    animationAddFrame(
        &animation,
        "bird_blue3"
    );
    animationAddFrame(
        &animation,
        "bird_blue4"
    );
    animationAddFrame(
        &animation,
        "bird_blue5"
    );
    animationAddFrame(
        &animation,
        "bird_blue6"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = birdAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(bird_purple){
    addEnemyBaseComponents(
        aabbMakeRadius(birdHitboxRadius),
        bird_purple1,
        spawn_explode_enemy
    );
    addRotateSpriteForward(componentListPtr);
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "bird_purple1"
    );
    animationAddFrame(
        &animation,
        "bird_purple2"
    );
    animationAddFrame(
        &animation,
        "bird_purple3"
    );
    animationAddFrame(
        &animation,
        "bird_purple4"
    );
    animationAddFrame(
        &animation,
        "bird_purple5"
    );
    animationAddFrame(
        &animation,
        "bird_purple6"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = birdAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(machine){
    addEnemyBaseComponents(
        aabbMakeRadius(machineHitboxRadius),
        machine1,
        spawn_explode_enemy
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "machine1"
    );
    animationAddFrame(
        &animation,
        "machine2"
    );
    animationAddFrame(
        &animation,
        "machine3"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = machineAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(firefly_left){
    addEnemyBaseComponents(
        aabbMakeRadius(fireflyHitboxRadius),
        firefly_left1,
        spawn_explode_enemy
    );
    addRotateSpriteForward(componentListPtr);
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "firefly_left1"
    );
    animationAddFrame(
        &animation,
        "firefly_left2"
    );
    animationAddFrame(
        &animation,
        "firefly_left3"
    );
    animationAddFrame(
        &animation,
        "firefly_left4"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = fireflyAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(firefly_right){
    addEnemyBaseComponents(
        aabbMakeRadius(fireflyHitboxRadius),
        firefly_right1,
        spawn_explode_enemy
    );
    addRotateSpriteForward(componentListPtr);
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "firefly_right1"
    );
    animationAddFrame(
        &animation,
        "firefly_right2"
    );
    animationAddFrame(
        &animation,
        "firefly_right3"
    );
    animationAddFrame(
        &animation,
        "firefly_right4"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = fireflyAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(bee){
    addEnemyBaseComponents(
        aabbMakeRadius(beeHitboxRadius),
        bee1,
        spawn_explode_enemy
    );
    addRotateSpriteForward(componentListPtr);
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "bee1"
    );
    animationAddFrame(
        &animation,
        "bee2"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = beeAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(plant){
    addEnemyBaseComponents(
        plantHitbox,
        plant1,
        spawn_explode_enemy
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "plant1"
    );
    animationAddFrame(
        &animation,
        "plant2"
    );
    animationAddFrame(
        &animation,
        "plant3"
    );
    animationAddFrame(
        &animation,
        "plant4"
    );
    animationAddFrame(
        &animation,
        "plant5"
    );
    animationAddFrame(
        &animation,
        "plant6"
    );
    animationAddFrame(
        &animation,
        "plant7"
    );
    animationAddFrame(
        &animation,
        "plant8"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = plantAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(blob){
    addEnemyBaseComponents(
        aabbMakeRadius(blobHitboxRadius),
        blob1,
        spawn_explode_enemy
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "blob1"
    );
    animationAddFrame(
        &animation,
        "blob2"
    );
    animationAddFrame(
        &animation,
        "blob3"
    );
    animationAddFrame(
        &animation,
        "blob4"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = blobAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(automaton_blue){
    addEnemyBaseComponents(
        automatonHitbox,
        automaton_blue1,
        spawn_explode_enemy
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "automaton_blue1"
    );
    animationAddFrame(
        &animation,
        "automaton_blue2"
    );
    animationAddFrame(
        &animation,
        "automaton_blue3"
    );
    animationAddFrame(
        &animation,
        "automaton_blue4"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = automatonAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(automaton_red){
    addEnemyBaseComponents(
        automatonHitbox,
        automaton_red1,
        spawn_explode_enemy
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "automaton_red1"
    );
    animationAddFrame(
        &animation,
        "automaton_red2"
    );
    animationAddFrame(
        &animation,
        "automaton_red3"
    );
    animationAddFrame(
        &animation,
        "automaton_red4"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = automatonAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(wisp){
    addEnemyBaseComponents(
        aabbMakeRadius(wispHitboxRadius),
        wisp1,
        spawn_explode_enemy
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "wisp1"
    );
    animationAddFrame(
        &animation,
        "wisp2"
    );
    animationAddFrame(
        &animation,
        "wisp3"
    );
    animationAddFrame(
        &animation,
        "wisp4"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = wispAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(crystal){
    addEnemyBaseComponents(
        crystalHitbox,
        crystal1,
        spawn_explode_enemy
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(
        &animation,
        "crystal1"
    );
    animationAddFrame(
        &animation,
        "crystal2"
    );
    animationAddFrame(
        &animation,
        "crystal3"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = crystalAnimationMaxTick;
    addAnimations(componentListPtr, &animations);
}

/* adds the basic components for a boss */
#define addBossBaseComponents( \
    SPRITEPREFIX \
) \
    do{ \
        addVisible(componentListPtr); \
        /* not collidable at first */ \
        addHitbox(componentListPtr, bossHitbox); \
        addPlayerCollisionSource( \
            componentListPtr, \
            collision_none \
        ); \
        addEnemyCollisionTarget( \
            componentListPtr, \
            collision_damage \
        ); \
        addDamage(componentListPtr, 1); \
        addHealth( \
            componentListPtr, \
            enemySpawnHealth \
        ); \
        addSpriteInstructionSimple( \
            componentListPtr, \
            gamePtr, \
            #SPRITEPREFIX "_idle1", \
            config_enemyDepth + depthOffset, \
            (Vector2D){0} \
        ); \
        Animations animations = animationListMake(); \
        Animation animation = animationMake(true); \
        animationAddFrame( \
            &animation, \
            #SPRITEPREFIX "_idle1" \
        ); \
        animationAddFrame( \
            &animation, \
            #SPRITEPREFIX "_idle2" \
        ); \
        animationAddFrame( \
            &animation, \
            #SPRITEPREFIX "_idle3" \
        ); \
        animationAddFrame( \
            &animation, \
            #SPRITEPREFIX "_idle4" \
        ); \
        arrayListPushBack(Animation, \
            &(animations.animations), \
            animation \
        ); \
        animations.currentIndex = 0; \
        animations.idleIndex = 0; \
        animations._maxTick = bossAnimationMaxTick; \
        addAnimations(componentListPtr, &animations); \
        addDeathCommand( \
            componentListPtr, \
            death_boss \
        ); \
        addDeathScripts( \
            componentListPtr, \
            ((DeathScripts){ \
                .scriptID1 = stringMakeC( \
                    "remove_ghost" \
                ), \
                .scriptID3 = stringMakeC( \
                    "clear_bullets" \
                ) \
            }) \
        ); \
    } while(false)

#define DECLARE_BOSS_PROTOTYPE(NAME, SPRITEPREFIX) \
    DECLARE_PROTOTYPE(NAME){ \
        addBossBaseComponents(SPRITEPREFIX); \
    } \

DECLARE_BOSS_PROTOTYPE(boss1, b1)
DECLARE_BOSS_PROTOTYPE(boss2, b2)
DECLARE_BOSS_PROTOTYPE(boss3, b3)
DECLARE_BOSS_PROTOTYPE(boss4, b4)


DECLARE_PROTOTYPE(life_wisp){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbMakeRadius(pickupWispHitboxRadius)
    );
    addEnemyCollisionTarget(
        componentListPtr,
        collision_damage
    );
    /* take damage from player on collision */
    addPickupCollisionSource(
        componentListPtr,
        collision_damage
    );
    addHealth(componentListPtr, enemySpawnHealth);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "life_wisp1",
        config_enemyDepth + depthOffset,
        ((Vector2D){0})
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(&animation, "life_wisp1");
    animationAddFrame(&animation, "life_wisp2");
    animationAddFrame(&animation, "life_wisp3");
    animationAddFrame(&animation, "life_wisp4");
    animationAddFrame(&animation, "life_wisp5");
    animationAddFrame(&animation, "life_wisp6");
    animationAddFrame(&animation, "life_wisp7");
    animationAddFrame(&animation, "life_wisp8");
    animationAddFrame(&animation, "life_wisp9");
    animationAddFrame(&animation, "life_wisp10");
    animationAddFrame(&animation, "life_wisp11");
    animationAddFrame(&animation, "life_wisp12");
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = pickupWispAnimationMaxTick;
    addAnimations(componentListPtr, &animations);

    addOutbound(componentListPtr, enemyOutbound);
    addDeathCommand(
        componentListPtr,
        death_script
    );
    addDeathScripts(
        componentListPtr,
        ((DeathScripts){
            .scriptID1 = stringMakeC(
                "remove_ghost"
            ),
            .scriptID2 = stringMakeC("add_life"),
            .scriptID3 = stringMakeC(
                "spawn_explode_bomb"
            )
        })
    );
}

DECLARE_PROTOTYPE(bomb_wisp){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbMakeRadius(pickupWispHitboxRadius)
    );
    addEnemyCollisionTarget(
        componentListPtr,
        collision_damage
    );
    /* take damage from player on collision */
    addPickupCollisionSource(
        componentListPtr,
        collision_damage
    );
    addHealth(componentListPtr, enemySpawnHealth);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "bomb_wisp1",
        config_enemyDepth + depthOffset,
        ((Vector2D){0})
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(&animation, "bomb_wisp1");
    animationAddFrame(&animation, "bomb_wisp2");
    animationAddFrame(&animation, "bomb_wisp3");
    animationAddFrame(&animation, "bomb_wisp4");
    animationAddFrame(&animation, "bomb_wisp5");
    animationAddFrame(&animation, "bomb_wisp6");
    animationAddFrame(&animation, "bomb_wisp7");
    animationAddFrame(&animation, "bomb_wisp8");
    animationAddFrame(&animation, "bomb_wisp9");
    animationAddFrame(&animation, "bomb_wisp10");
    animationAddFrame(&animation, "bomb_wisp11");
    animationAddFrame(&animation, "bomb_wisp12");
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = pickupWispAnimationMaxTick;
    addAnimations(componentListPtr, &animations);

    addOutbound(componentListPtr, enemyOutbound);
    addDeathCommand(
        componentListPtr,
        death_script
    );
    addDeathScripts(
        componentListPtr,
        ((DeathScripts){
            .scriptID1 = stringMakeC(
                "remove_ghost"
            ),
            .scriptID2 = stringMakeC("add_bomb"),
            .scriptID3 = stringMakeC(
                "spawn_explode_bomb"
            )
        })
    );
}

/* BULLET PROTOTYPES */

/* adds the basic components for a bullet */
#define addBulletBaseComponents( \
    HITBOX, \
    SPRITEID, \
    BASEDEPTH \
) \
    do{ \
        addVisible(componentListPtr); \
        addCollidable(componentListPtr); \
        addHitbox(componentListPtr, HITBOX); \
        addPlayerCollisionSource( \
            componentListPtr, \
            collision_death \
        ); \
        addBulletCollisionTarget( \
            componentListPtr, \
            collision_death \
        ); \
        addClearable(componentListPtr); \
        addDamage(componentListPtr, 1); \
        addSpriteInstructionSimple( \
            componentListPtr, \
            gamePtr, \
            #SPRITEID, \
            BASEDEPTH + depthOffset, \
            (Vector2D){0} \
        ); \
        addOutbound( \
            componentListPtr, \
            bulletOutbound \
        ); \
        addDeathCommand( \
            componentListPtr, \
            death_script \
        ); \
        addDeathScripts( \
            componentListPtr, \
            ((DeathScripts){ \
                .scriptID1 = stringMakeC( \
                    "remove_ghost" \
                ), \
                .scriptID3 = stringMakeC( \
                    "spawn_explode_projectile" \
                ) \
            }) \
        ); \
    } while(false);

#define DECLARE_BULLET_PROTOTYPE( \
    NAME, \
    HITBOX, \
    SPRITEID, \
    BASEDEPTH \
) \
    DECLARE_PROTOTYPE(NAME){ \
        addBulletBaseComponents( \
            HITBOX, \
            SPRITEID, \
            BASEDEPTH \
        ); \
    } \

#define DECLARE_POINTED_BULLET_PROTOTYPE( \
    NAME, \
    HITBOX, \
    SPRITEID, \
    BASEDEPTH \
) \
    DECLARE_PROTOTYPE(NAME){ \
        addBulletBaseComponents( \
            HITBOX, \
            SPRITEID, \
            BASEDEPTH \
        ); \
        addRotateSpriteForward(componentListPtr); \
    } \

#define DECLARE_LASER_PROTOTYPE( \
    NAME, \
    HITBOX, \
    SPRITEID, \
    BASEDEPTH \
) \
    DECLARE_PROTOTYPE(NAME){ \
        addVisible(componentListPtr); \
        /* lasers not immediately collidable */ \
        addHitbox(componentListPtr, HITBOX); \
        /* lasers do not die upon hit */ \
        addPlayerCollisionSource( \
            componentListPtr, \
            collision_none \
        ); \
        addBulletCollisionTarget( \
            componentListPtr, \
            collision_none \
        ); \
        addClearable(componentListPtr); \
        addDamage(componentListPtr, 1); \
        addSpriteInstructionSimple( \
            componentListPtr, \
            gamePtr, \
            "spawn_laser1", \
            BASEDEPTH + depthOffset, \
            (Vector2D){0} \
        ); \
        addOutbound( \
            componentListPtr, \
            bulletOutbound \
        ); \
        addDeathCommand( \
            componentListPtr, \
            death_script \
        ); \
        addDeathScripts( \
            componentListPtr, \
            ((DeathScripts){ \
                .scriptID1 = stringMakeC( \
                    "remove_ghost" \
                ), \
                .scriptID3 = stringMakeC( \
                    "spawn_explode_laser" \
                ) \
            }) \
        ); \
        addRotateSpriteForward(componentListPtr); \
        Animations animations = animationListMake(); \
        Animation animation = animationMake(false); \
        animationAddFrame( \
            &animation, \
            "spawn_laser1" \
        ); \
        animationAddFrame( \
            &animation, \
            "spawn_laser2" \
        ); \
        animationAddFrame( \
            &animation, \
            "spawn_laser3" \
        ); \
        animationAddFrame( \
            &animation, \
            #SPRITEID \
        ); \
        arrayListPushBack(Animation, \
            &(animations.animations), \
            animation \
        ); \
        animations.currentIndex = 0; \
        animations.idleIndex = 0; \
        animations._maxTick = 15; \
        addAnimations(componentListPtr, &animations); \
    }

#define DECLARE_STAR_PROTOTYPE( \
    NAME, \
    HITBOX, \
    SPRITEID, \
    BASEDEPTH \
) \
    DECLARE_PROTOTYPE(NAME){ \
        addBulletBaseComponents( \
            HITBOX, \
            SPRITEID, \
            BASEDEPTH \
        ); \
        float spin = 0; \
        if(zmtRandBool(&scenePtr->messages.prng)){ \
            spin = starSpin; \
        } \
        else{ \
            spin = -starSpin; \
        } \
        addSpriteSpin(componentListPtr, spin); \
    } \

#define DECLARE_ALL_COLORS( \
    NAMEPREFIX, \
    HITBOX, \
    SPRITEIDPREFIX, \
    BASEDEPTH, \
    DECLAREMACRO \
) \
    DECLAREMACRO( \
        NAMEPREFIX##_black, \
        HITBOX, \
        SPRITEIDPREFIX##_black, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_blue, \
        HITBOX, \
        SPRITEIDPREFIX##_blue, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_brown, \
        HITBOX, \
        SPRITEIDPREFIX##_brown, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_clear, \
        HITBOX, \
        SPRITEIDPREFIX##_clear, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_dgreen, \
        HITBOX, \
        SPRITEIDPREFIX##_dgreen, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_dpurple, \
        HITBOX, \
        SPRITEIDPREFIX##_dpurple, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_gray, \
        HITBOX, \
        SPRITEIDPREFIX##_gray, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_lblue, \
        HITBOX, \
        SPRITEIDPREFIX##_lblue, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_lgreen, \
        HITBOX, \
        SPRITEIDPREFIX##_lgreen, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_lpurple, \
        HITBOX, \
        SPRITEIDPREFIX##_lpurple, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_lyellow, \
        HITBOX, \
        SPRITEIDPREFIX##_lyellow, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_orange, \
        HITBOX, \
        SPRITEIDPREFIX##_orange, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_purple, \
        HITBOX, \
        SPRITEIDPREFIX##_purple, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_red, \
        HITBOX, \
        SPRITEIDPREFIX##_red, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_white, \
        HITBOX, \
        SPRITEIDPREFIX##_white, \
        BASEDEPTH \
    ) \
    DECLAREMACRO( \
        NAMEPREFIX##_yellow, \
        HITBOX, \
        SPRITEIDPREFIX##_yellow, \
        BASEDEPTH \
    )

DECLARE_ALL_COLORS(
    small,
    aabbMakeRadius(smallHitboxRadius),
    small,
    config_enemyBulletDepth3,
    DECLARE_BULLET_PROTOTYPE
)

DECLARE_ALL_COLORS(
    medium,
    aabbMakeRadius(mediumHitboxRadius),
    medium,
    config_enemyBulletDepth2,
    DECLARE_BULLET_PROTOTYPE
)

DECLARE_ALL_COLORS(
    large,
    aabbMakeRadius(largeHitboxRadius),
    large,
    config_enemyBulletDepth0,
    DECLARE_BULLET_PROTOTYPE
)

DECLARE_ALL_COLORS(
    sharp,
    aabbMakeRadius(sharpHitboxRadius),
    sharp,
    config_enemyBulletDepth4,
    DECLARE_POINTED_BULLET_PROTOTYPE
)

DECLARE_ALL_COLORS(
    laser,
    aabbMakeRadius(laserHitboxRadius),
    laser,
    config_enemyBulletDepth1,
    DECLARE_LASER_PROTOTYPE
)

DECLARE_ALL_COLORS(
    star,
    aabbMakeRadius(starHitboxRadius),
    star,
    config_enemyBulletDepth3,
    DECLARE_STAR_PROTOTYPE
)

/* MISCELLANEOUS PROTOTYPES */

DECLARE_PROTOTYPE(explode_projectile){
    addVisible(componentListPtr);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "explode_projectile1",
        config_effectDepth + depthOffset,
        (Vector2D){0}
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(
        &animation,
        "explode_projectile1"
    );
    animationAddFrame(
        &animation,
        "explode_projectile2"
    );
    animationAddFrame(
        &animation,
        "explode_projectile3"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 3;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(explode_enemy){
    addVisible(componentListPtr);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "explode_enemy1",
        config_effectDepth + depthOffset,
        (Vector2D){0}
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(
        &animation,
        "explode_enemy1"
    );
    animationAddFrame(
        &animation,
        "explode_enemy2"
    );
    animationAddFrame(
        &animation,
        "explode_enemy3"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 3;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(explode_boss){
    addVisible(componentListPtr);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "explode_boss1",
        config_effectDepth + depthOffset,
        (Vector2D){0}
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(
        &animation,
        "explode_boss1"
    );
    animationAddFrame(
        &animation,
        "explode_boss2"
    );
    animationAddFrame(
        &animation,
        "explode_boss3"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 4;
    addAnimations(componentListPtr, &animations);
}

DECLARE_PROTOTYPE(explode_laser){
    addVisible(componentListPtr);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "explode_laser1",
        config_effectDepth + depthOffset,
        (Vector2D){0}
    );
    addRotateSpriteForward(componentListPtr);
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(
        &animation,
        "explode_laser1"
    );
    animationAddFrame(
        &animation,
        "explode_laser2"
    );
    animationAddFrame(
        &animation,
        "explode_laser3"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 3;
    addAnimations(componentListPtr, &animations);
}

/* map from char* (unowned) to PrototypeFunction */
static HashMap prototypeFunctionMap;
static bool initialized = false;

/*
 * piggyback off the system destructor system to clean
 * up the prototype function map
 */
static void destroy(){
    if(initialized){
        hashMapFree(char*, PrototypeFunction,
            &prototypeFunctionMap
        );
        initialized = false;
    }
}

/*
 * piggyback off the system destructor system to init
 * the prototype function map
 */
static void init(){
    if(!initialized){
        prototypeFunctionMap = hashMapMake(
            char*,
            PrototypeFunction,
            50,
            cStringHash,
            cStringEquals
        );

        #define addPrototypeFunction(FUNCNAME) \
            hashMapPut(char*, PrototypeFunction, \
                &(prototypeFunctionMap), \
                #FUNCNAME, \
                FUNCNAME \
            )

        /* player prototypes */
        addPrototypeFunction(shard);
        addPrototypeFunction(caltrop);
        addPrototypeFunction(spike_bottomLeft);
        addPrototypeFunction(spike_bottomRight);
        addPrototypeFunction(spike_topLeft);
        addPrototypeFunction(spike_topRight);
        addPrototypeFunction(bomb);
        addPrototypeFunction(explode_bomb);

        /* pickup prototypes */
        addPrototypeFunction(power_small);
        addPrototypeFunction(power_large);

        /* enemy prototypes */
        addPrototypeFunction(star_medium);
        addPrototypeFunction(star_large);
        addPrototypeFunction(bird_blue);
        addPrototypeFunction(bird_purple);
        addPrototypeFunction(machine);
        addPrototypeFunction(firefly_left);
        addPrototypeFunction(firefly_right);
        addPrototypeFunction(bee);
        addPrototypeFunction(plant);
        addPrototypeFunction(blob);
        addPrototypeFunction(automaton_blue);
        addPrototypeFunction(automaton_red);
        addPrototypeFunction(wisp);
        addPrototypeFunction(crystal);
        addPrototypeFunction(boss1);
        addPrototypeFunction(boss2);
        addPrototypeFunction(boss3);
        addPrototypeFunction(boss4);
        addPrototypeFunction(life_wisp);
        addPrototypeFunction(bomb_wisp);

        /* bullet prototypes */
        #define addAllColors(NAMEPREFIX) \
            do{ \
                addPrototypeFunction( \
                    NAMEPREFIX##_black \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_blue \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_brown \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_clear \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_dgreen \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_dpurple \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_gray \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_lblue \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_lgreen \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_lpurple \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_lyellow \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_orange \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_purple \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_red \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_white \
                ); \
                addPrototypeFunction( \
                    NAMEPREFIX##_yellow \
                ); \
            } while(false)

        addAllColors(small);
        addAllColors(medium);
        addAllColors(large);
        addAllColors(sharp);
        addAllColors(laser);
        addAllColors(star);

        #undef addAllColors

        /* miscellaneous prototypes */
        addPrototypeFunction(explode_projectile);
        addPrototypeFunction(explode_enemy);
        addPrototypeFunction(explode_boss);
        addPrototypeFunction(explode_laser);

        #undef addPrototypeFunction

        initialized = true;
    }
}

/*
 * Applies the specified prototype to the given
 * component list array; also applies the depth
 * offset when creating the sprite instruction
 */
void applyPrototype(
    Game *gamePtr,
    Scene *scenePtr,
    String *prototypeIDPtr,
    ArrayList *componentListPtr,
    int depthOffset
){
    init();

    assertNotNull(
        gamePtr,
        "null game passed;" SRC_LOCATION
    );
    assertNotNull(
        scenePtr,
        "null scene passed;" SRC_LOCATION
    );
    assertNotNull(
        prototypeIDPtr,
        "null prototype ID passed;" SRC_LOCATION
    );
    assertNotNull(
        prototypeIDPtr->_ptr,
        "prototype string holds null;" SRC_LOCATION
    );
    
    if(!hashMapHasKey(char*, PrototypeFunction,
        &(prototypeFunctionMap),
        prototypeIDPtr->_ptr
    )){
        pgWarning(prototypeIDPtr->_ptr);
        pgError(
            "failed to find prototype; " SRC_LOCATION
        );
    }
    PrototypeFunction prototypeFunction = hashMapGet(
        char*,
        PrototypeFunction,
        &(prototypeFunctionMap),
        prototypeIDPtr->_ptr
    );

    prototypeFunction(
        gamePtr,
        scenePtr,
        componentListPtr,
        depthOffset
    );
}