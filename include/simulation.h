#ifndef DEEP_SIMULATION_H
#define DEEP_SIMULATION_H

#include "game.h"

typedef struct {
    BigNum yields[RES_COUNT];
    int hazard_triggered; /* 0 = none, 1 = hazard occurred, -1 = mining blocked (tech gate) */
    double depth_gained;
    double efficiency; /* 1.0 = full effectiveness; lower when life support is degraded */
} MiningResult;

MiningResult simulation_do_manual_mine(GameState *g);
void simulation_apply_offline_progress(GameState *g, long elapsed_seconds);

/* Called once per loop (and once at startup) to let planetary
 * mining stations produce continuously in real time. */
void simulation_tick_real_time(GameState *g);

#endif
