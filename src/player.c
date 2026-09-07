#include "player.h"

void player_init(Player *p) {
    resource_store_init(&p->resources);
    p->current_planet_id = 0;
    p->total_meters_mined = 0.0;
    p->playtime_seconds = 0.0;
    /* small bootstrap stockpile so the tutorial planet isn't a dead stop */
    p->resources.amount[RES_IRON_ORE] = bn_from_double(20);
}
