#include "game.h"
#include <string.h>

void game_new(GameState *g) {
    memset(g, 0, sizeof(*g));
    g->magic = SAVE_MAGIC;
    g->version = SAVE_VERSION;
    player_init(&g->player);
    planet_registry_init_default(&g->planets);
    ship_init_default(&g->ship);
    research_state_init(&g->research);
    discovery_state_init(&g->discoveries);
    prestige_state_init(&g->prestige);
    g->last_tick_time = time(NULL);
    g->automation_unlocked = 0;
    g->running = 1;
}

Planet *game_current_planet(GameState *g) {
    int i;
    for (i = 0; i < g->planets.count; i++) {
        if (g->planets.planets[i].id == g->player.current_planet_id) return &g->planets.planets[i];
    }
    return &g->planets.planets[0];
}
