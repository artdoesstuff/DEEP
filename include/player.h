#ifndef DEEP_PLAYER_H
#define DEEP_PLAYER_H

#include "resources.h"

typedef struct {
    ResourceStore resources;
    int current_planet_id;
    double total_meters_mined;
    double playtime_seconds;
} Player;

void player_init(Player *p);

#endif
