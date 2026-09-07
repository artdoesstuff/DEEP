#ifndef DEEP_GAME_H
#define DEEP_GAME_H

#include <time.h>
#include "player.h"
#include "planet.h"
#include "ship.h"
#include "research.h"
#include "discovery.h"
#include "prestige.h"

#define SAVE_MAGIC 0x44454550u /* "DEEP" */
#define SAVE_VERSION 3

typedef struct {
    unsigned int magic;
    int version;
    Player player;
    PlanetRegistry planets;
    Ship ship;
    ResearchState research;
    DiscoveryState discoveries;
    PrestigeState prestige;
    time_t last_tick_time;
    int automation_unlocked;
    int running;
} GameState;

void game_new(GameState *g);
Planet *game_current_planet(GameState *g);

#endif
