#ifndef DEEP_DISCOVERY_H
#define DEEP_DISCOVERY_H

#define MAX_DISCOVERIES 64

typedef struct {
    int id;
    char name[64];
    char text[400];
    int min_layer;       /* LayerType ordinal required to trigger this discovery */
    double rarity;       /* base roll chance per mining action once eligible */
    int discovered;
    int planet_specific; /* -1 = any planet, else a specific planet id */
} Discovery;

typedef struct {
    Discovery items[MAX_DISCOVERIES];
    int count;
    int total_discovered;
} DiscoveryState;

void discovery_state_init(DiscoveryState *ds);
Discovery *discovery_try_roll(DiscoveryState *ds, int planet_id, int layer, double scanner_power);
Discovery *discovery_find(DiscoveryState *ds, const char *name);

#endif
