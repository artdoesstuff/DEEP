#include "planet.h"
#include <string.h>
#include <strings.h>

static void zero_weights(double w[RES_COUNT]) {
    int i;
    for (i = 0; i < RES_COUNT; i++) w[i] = 0.0;
}

static void set_layer(PlanetLayer *l, double minD, double maxD, double temp, double pressure,
                       double hazard, double energy_mult, double speed_mult) {
    l->min_depth_m = minD;
    l->max_depth_m = maxD;
    l->base_temp_k = temp;
    l->base_pressure_pa = pressure;
    l->hazard_chance = hazard;
    l->energy_cost_mult = energy_mult;
    l->mining_speed_mult = speed_mult;
    zero_weights(l->resource_weight);
}

static void init_common_layers(Planet *p, double surf_t, double surf_p) {
    set_layer(&p->layers[LAYER_SURFACE],    0,        100,      surf_t,       surf_p,          0.00, 1.0,  1.2);
    set_layer(&p->layers[LAYER_CRUST],      100,      10000,    surf_t+50,    surf_p*1e3,       0.01, 1.3,  1.0);
    set_layer(&p->layers[LAYER_DEEP_CRUST], 10000,    50000,    surf_t+400,   surf_p*1e6,       0.04, 1.8,  0.8);
    set_layer(&p->layers[LAYER_MANTLE],     50000,    500000,   surf_t+2000,  surf_p*1e9,       0.10, 2.6,  0.55);
    set_layer(&p->layers[LAYER_CORE],       500000,   2000000,  surf_t+5500,  surf_p*1e11,      0.20, 4.0,  0.35);
    set_layer(&p->layers[LAYER_UNKNOWN],    2000000,  10000000, surf_t+9000,  surf_p*1e13,      0.35, 7.0,  0.20);
    set_layer(&p->layers[LAYER_IMPOSSIBLE], 10000000, -1,       surf_t+20000, surf_p*1e15,      0.55, 15.0, 0.10);
}

void planet_registry_init_default(PlanetRegistry *reg) {
    memset(reg, 0, sizeof(*reg));
    int idx = 0;
    Planet *p;
    int li;

    p = &reg->planets[idx];
    memset(p, 0, sizeof(*p));
    p->id = idx;
    strncpy(p->name, "Terra", sizeof(p->name) - 1);
    strncpy(p->description,
        "Homeworld. Temperate, geologically stable, abundant common resources. The safest place to learn to dig.",
        sizeof(p->description) - 1);
    p->unlocked = 1; p->discovered = 1;
    p->system_id = 0; p->distance_from_sun_au = 1.0; p->min_tech_tier_to_unlock = 0;
    init_common_layers(p, 288, 101325);
    p->layers[LAYER_SURFACE].resource_weight[RES_IRON_ORE] = 0.30;
    p->layers[LAYER_SURFACE].resource_weight[RES_SILICON] = 0.25;
    p->layers[LAYER_SURFACE].resource_weight[RES_WATER] = 0.30;
    p->layers[LAYER_SURFACE].resource_weight[RES_CARBON] = 0.15;
    p->layers[LAYER_CRUST].resource_weight[RES_IRON_ORE] = 0.35;
    p->layers[LAYER_CRUST].resource_weight[RES_SILICON] = 0.30;
    p->layers[LAYER_CRUST].resource_weight[RES_ALUMINUM] = 0.20;
    p->layers[LAYER_CRUST].resource_weight[RES_WATER] = 0.15;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_IRON_ORE] = 0.30;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_TITANIUM] = 0.30;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_RARE_METALS] = 0.20;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_ALUMINUM] = 0.20;
    p->layers[LAYER_MANTLE].resource_weight[RES_IRON_ORE] = 0.50;
    p->layers[LAYER_MANTLE].resource_weight[RES_RARE_METALS] = 0.30;
    p->layers[LAYER_MANTLE].resource_weight[RES_TITANIUM] = 0.20;
    p->layers[LAYER_CORE].resource_weight[RES_IRON_ORE] = 0.60;
    p->layers[LAYER_CORE].resource_weight[RES_RARE_METALS] = 0.40;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_RARE_METALS] = 0.50;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_UNKNOWN_MATERIAL] = 0.50;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.70;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_EXOTIC_MATTER] = 0.30;
    idx++;

    p = &reg->planets[idx];
    memset(p, 0, sizeof(*p));
    p->id = idx;
    strncpy(p->name, "Luna", sizeof(p->name) - 1);
    strncpy(p->description,
        "Low-gravity moon. Rich Helium-3 in the regolith, and a maze of underground caverns beneath the maria.",
        sizeof(p->description) - 1);
    p->unlocked = 1; p->discovered = 0;
    p->system_id = 0; p->distance_from_sun_au = 1.0; p->min_tech_tier_to_unlock = 0;
    init_common_layers(p, 220, 0.0000001);
    for (li = 0; li < NUM_LAYERS; li++) p->layers[li].mining_speed_mult *= 1.3; /* low gravity */
    p->layers[LAYER_SURFACE].resource_weight[RES_HELIUM3] = 0.60;
    p->layers[LAYER_SURFACE].resource_weight[RES_SILICON] = 0.40;
    p->layers[LAYER_CRUST].resource_weight[RES_HELIUM3] = 0.40;
    p->layers[LAYER_CRUST].resource_weight[RES_IRON_ORE] = 0.30;
    p->layers[LAYER_CRUST].resource_weight[RES_ALUMINUM] = 0.30;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_TITANIUM] = 0.40;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_RARE_METALS] = 0.30;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_HELIUM3] = 0.30;
    p->layers[LAYER_MANTLE].resource_weight[RES_RARE_METALS] = 0.50;
    p->layers[LAYER_MANTLE].resource_weight[RES_IRON_ORE] = 0.50;
    p->layers[LAYER_CORE].resource_weight[RES_IRON_ORE] = 0.70;
    p->layers[LAYER_CORE].resource_weight[RES_RARE_METALS] = 0.30;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_UNKNOWN_MATERIAL] = 0.60;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_RARE_METALS] = 0.40;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.60;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_EXOTIC_MATTER] = 0.40;
    idx++;

    p = &reg->planets[idx];
    memset(p, 0, sizeof(*p));
    p->id = idx;
    strncpy(p->name, "Mars", sizeof(p->name) - 1);
    strncpy(p->description,
        "Rust-red deserts hide rare metal veins. Dust storms disrupt surface operations. Something older than the dunes waits beneath the crust.",
        sizeof(p->description) - 1);
    p->unlocked = 1; p->discovered = 0;
    p->system_id = 0; p->distance_from_sun_au = 1.52; p->min_tech_tier_to_unlock = 1;
    init_common_layers(p, 210, 610);
    for (li = 0; li < NUM_LAYERS; li++) p->layers[li].hazard_chance += 0.03; /* dust storms */
    p->layers[LAYER_SURFACE].resource_weight[RES_IRON_ORE] = 0.40;
    p->layers[LAYER_SURFACE].resource_weight[RES_SILICON] = 0.30;
    p->layers[LAYER_SURFACE].resource_weight[RES_RARE_METALS] = 0.30;
    p->layers[LAYER_CRUST].resource_weight[RES_RARE_METALS] = 0.45;
    p->layers[LAYER_CRUST].resource_weight[RES_IRON_ORE] = 0.35;
    p->layers[LAYER_CRUST].resource_weight[RES_TITANIUM] = 0.20;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_RARE_METALS] = 0.50;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_TITANIUM] = 0.30;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_UNKNOWN_MATERIAL] = 0.20;
    p->layers[LAYER_MANTLE].resource_weight[RES_RARE_METALS] = 0.60;
    p->layers[LAYER_MANTLE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.40;
    p->layers[LAYER_CORE].resource_weight[RES_RARE_METALS] = 0.50;
    p->layers[LAYER_CORE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.50;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_UNKNOWN_MATERIAL] = 0.70;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_EXOTIC_MATTER] = 0.30;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_EXOTIC_MATTER] = 0.60;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.40;
    idx++;

    p = &reg->planets[idx];
    memset(p, 0, sizeof(*p));
    p->id = idx;
    strncpy(p->name, "Venus", sizeof(p->name) - 1);
    strncpy(p->description,
        "Crushing pressure and furnace heat. Equipment degrades fast here, but what it pulls up is extraordinarily valuable.",
        sizeof(p->description) - 1);
    p->unlocked = 1; p->discovered = 0;
    p->system_id = 0; p->distance_from_sun_au = 0.72; p->min_tech_tier_to_unlock = 1;
    init_common_layers(p, 737, 9200000);
    for (li = 0; li < NUM_LAYERS; li++) {
        p->layers[li].hazard_chance += 0.10;
        p->layers[li].energy_cost_mult *= 1.8;
        p->layers[li].mining_speed_mult *= 0.7;
    }
    p->layers[LAYER_SURFACE].resource_weight[RES_TITANIUM] = 0.40;
    p->layers[LAYER_SURFACE].resource_weight[RES_RARE_METALS] = 0.40;
    p->layers[LAYER_SURFACE].resource_weight[RES_SILICON] = 0.20;
    p->layers[LAYER_CRUST].resource_weight[RES_TITANIUM] = 0.40;
    p->layers[LAYER_CRUST].resource_weight[RES_RARE_METALS] = 0.40;
    p->layers[LAYER_CRUST].resource_weight[RES_ALUMINUM] = 0.20;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_RARE_METALS] = 0.60;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_TITANIUM] = 0.40;
    p->layers[LAYER_MANTLE].resource_weight[RES_RARE_METALS] = 0.70;
    p->layers[LAYER_MANTLE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.30;
    p->layers[LAYER_CORE].resource_weight[RES_RARE_METALS] = 0.50;
    p->layers[LAYER_CORE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.50;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_UNKNOWN_MATERIAL] = 0.60;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_EXOTIC_MATTER] = 0.40;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_EXOTIC_MATTER] = 0.70;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.30;
    idx++;

    p = &reg->planets[idx];
    memset(p, 0, sizeof(*p));
    p->id = idx;
    strncpy(p->name, "Europa", sizeof(p->name) - 1);
    strncpy(p->description,
        "Ice moon of Jupiter. A vast subsurface ocean lies beneath the frozen shell. Sensors keep flagging organic anomalies.",
        sizeof(p->description) - 1);
    p->unlocked = 1; p->discovered = 0;
    p->system_id = 0; p->distance_from_sun_au = 5.2; p->min_tech_tier_to_unlock = 2;
    init_common_layers(p, 102, 0.0000001);
    p->layers[LAYER_SURFACE].resource_weight[RES_WATER] = 0.80;
    p->layers[LAYER_SURFACE].resource_weight[RES_SILICON] = 0.20;
    p->layers[LAYER_CRUST].resource_weight[RES_WATER] = 0.70;
    p->layers[LAYER_CRUST].resource_weight[RES_CARBON] = 0.30;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_WATER] = 0.50;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_CARBON] = 0.30;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_UNKNOWN_MATERIAL] = 0.20;
    p->layers[LAYER_MANTLE].resource_weight[RES_WATER] = 0.40;
    p->layers[LAYER_MANTLE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.40;
    p->layers[LAYER_MANTLE].resource_weight[RES_RARE_METALS] = 0.20;
    p->layers[LAYER_CORE].resource_weight[RES_RARE_METALS] = 0.50;
    p->layers[LAYER_CORE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.50;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_UNKNOWN_MATERIAL] = 0.80;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_EXOTIC_MATTER] = 0.20;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_EXOTIC_MATTER] = 0.50;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.50;
    idx++;

    p = &reg->planets[idx];
    memset(p, 0, sizeof(*p));
    p->id = idx;
    strncpy(p->name, "Titan", sizeof(p->name) - 1);
    strncpy(p->description,
        "Saturn's largest moon. Thick nitrogen atmosphere, methane lakes, hydrocarbon-soaked surface. Bitterly cold.",
        sizeof(p->description) - 1);
    p->unlocked = 1; p->discovered = 0;
    p->system_id = 0; p->distance_from_sun_au = 9.5; p->min_tech_tier_to_unlock = 2;
    init_common_layers(p, 94, 146700);
    p->layers[LAYER_SURFACE].resource_weight[RES_HYDROCARBONS] = 0.70;
    p->layers[LAYER_SURFACE].resource_weight[RES_CARBON] = 0.30;
    p->layers[LAYER_CRUST].resource_weight[RES_HYDROCARBONS] = 0.60;
    p->layers[LAYER_CRUST].resource_weight[RES_WATER] = 0.40;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_HYDROCARBONS] = 0.40;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_WATER] = 0.40;
    p->layers[LAYER_DEEP_CRUST].resource_weight[RES_RARE_METALS] = 0.20;
    p->layers[LAYER_MANTLE].resource_weight[RES_WATER] = 0.50;
    p->layers[LAYER_MANTLE].resource_weight[RES_RARE_METALS] = 0.50;
    p->layers[LAYER_CORE].resource_weight[RES_RARE_METALS] = 0.60;
    p->layers[LAYER_CORE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.40;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_UNKNOWN_MATERIAL] = 0.70;
    p->layers[LAYER_UNKNOWN].resource_weight[RES_EXOTIC_MATTER] = 0.30;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_EXOTIC_MATTER] = 0.60;
    p->layers[LAYER_IMPOSSIBLE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.40;
    idx++;

    p = &reg->planets[idx];
    memset(p, 0, sizeof(*p));
    p->id = idx;
    strncpy(p->name, "Proxima b", sizeof(p->name) - 1);
    strncpy(p->description,
        "An exoplanet orbiting Proxima Centauri. Telemetry withheld until an interstellar-capable vessel arrives.",
        sizeof(p->description) - 1);
    p->unlocked = 0; p->discovered = 0;
    p->system_id = 1; p->distance_from_sun_au = 268000.0; p->min_tech_tier_to_unlock = 3;
    init_common_layers(p, 234, 50000);
    p->layers[LAYER_SURFACE].resource_weight[RES_RARE_METALS] = 0.5;
    p->layers[LAYER_SURFACE].resource_weight[RES_UNKNOWN_MATERIAL] = 0.5;
    idx++;

    p = &reg->planets[idx];
    memset(p, 0, sizeof(*p));
    p->id = idx;
    strncpy(p->name, "Proxima c", sizeof(p->name) - 1);
    strncpy(p->description,
        "A distant super-Earth in the Proxima system. Almost nothing is known about it yet.",
        sizeof(p->description) - 1);
    p->unlocked = 0; p->discovered = 0;
    p->system_id = 1; p->distance_from_sun_au = 270000.0; p->min_tech_tier_to_unlock = 3;
    init_common_layers(p, 40, 10000);
    p->layers[LAYER_SURFACE].resource_weight[RES_UNKNOWN_MATERIAL] = 1.0;
    idx++;

    p = &reg->planets[idx];
    memset(p, 0, sizeof(*p));
    p->id = idx;
    strncpy(p->name, "Kepler-186f", sizeof(p->name) - 1);
    strncpy(p->description,
        "A potentially habitable world some 500 light-years distant. Far beyond current propulsion range.",
        sizeof(p->description) - 1);
    p->unlocked = 0; p->discovered = 0;
    p->system_id = 2; p->distance_from_sun_au = 31000000.0; p->min_tech_tier_to_unlock = 4;
    init_common_layers(p, 188, 80000);
    p->layers[LAYER_SURFACE].resource_weight[RES_EXOTIC_MATTER] = 1.0;
    idx++;

    reg->count = idx;
}

LayerType planet_layer_at_depth(const Planet *p, double depth) {
    int i;
    for (i = 0; i < NUM_LAYERS; i++) {
        double maxd = p->layers[i].max_depth_m;
        if (maxd < 0) return (LayerType)i;
        if (depth < maxd) return (LayerType)i;
    }
    return LAYER_IMPOSSIBLE;
}

const char *layer_name(LayerType t) {
    switch (t) {
        case LAYER_SURFACE: return "Surface";
        case LAYER_CRUST: return "Crust";
        case LAYER_DEEP_CRUST: return "Deep Crust";
        case LAYER_MANTLE: return "Mantle";
        case LAYER_CORE: return "Core";
        case LAYER_UNKNOWN: return "Unknown Layer";
        case LAYER_IMPOSSIBLE: return "Impossible Depth";
        default: return "???";
    }
}

Planet *planet_find(PlanetRegistry *reg, const char *name) {
    int i;
    for (i = 0; i < reg->count; i++) {
        if (strcasecmp(reg->planets[i].name, name) == 0) return &reg->planets[i];
    }
    for (i = 0; i < reg->count; i++) {
        if (strncasecmp(reg->planets[i].name, name, strlen(name)) == 0) return &reg->planets[i];
    }
    return NULL;
}
