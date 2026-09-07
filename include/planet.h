#ifndef DEEP_PLANET_H
#define DEEP_PLANET_H

#include "bignum.h"
#include "resources.h"

#define MAX_PLANETS 16
#define NUM_LAYERS 7

typedef enum {
    LAYER_SURFACE = 0,
    LAYER_CRUST,
    LAYER_DEEP_CRUST,
    LAYER_MANTLE,
    LAYER_CORE,
    LAYER_UNKNOWN,
    LAYER_IMPOSSIBLE
} LayerType;

typedef struct {
    double min_depth_m;
    double max_depth_m;       /* -1 = unbounded (Impossible Depths) */
    double base_temp_k;
    double base_pressure_pa;
    double hazard_chance;     /* probability of a hazard event per mining action */
    double energy_cost_mult;
    double mining_speed_mult;
    double resource_weight[RES_COUNT]; /* relative abundance of each resource here */
} PlanetLayer;

typedef struct {
    int id;
    char name[32];
    char description[256];
    int unlocked;       /* can the ship currently travel here */
    int discovered;     /* has it been visited/surveyed */
    double current_depth_m;
    double max_depth_reached_m;
    PlanetLayer layers[NUM_LAYERS];
    int system_id;
    double distance_from_sun_au;
    int min_tech_tier_to_unlock; /* matches PropulsionType ordinal required */
    int station_level;  /* 0 = no automated infrastructure built here yet */
} Planet;

typedef struct {
    Planet planets[MAX_PLANETS];
    int count;
} PlanetRegistry;

void planet_registry_init_default(PlanetRegistry *reg);
LayerType planet_layer_at_depth(const Planet *p, double depth);
const char *layer_name(LayerType t);
Planet *planet_find(PlanetRegistry *reg, const char *name);

#endif
