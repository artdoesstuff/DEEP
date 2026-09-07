#include "ship.h"
#include <math.h>

void ship_init_default(Ship *s) {
    s->hull_level = 1;
    s->engine_level = 1;
    s->cargo_level = 1;
    s->mining_equipment_level = 1;
    s->scanner_level = 1;
    s->fuel_capacity = 100.0;
    s->fuel = 100.0;
    s->power_capacity = 100.0;
    s->power = 100.0;
    s->life_support = 100.0;
    s->equipment_wear = 0.0;
    s->propulsion = PROP_CHEMICAL;
}

double ship_cargo_capacity(const Ship *s) {
    return 1500.0 * pow(1.6, s->cargo_level - 1);
}

double ship_mining_power(const Ship *s) {
    return 1.0 * pow(1.5, s->mining_equipment_level - 1);
}

double ship_scan_power(const Ship *s) {
    return 1.0 * pow(1.4, s->scanner_level - 1);
}

const char *propulsion_name(PropulsionType p) {
    switch (p) {
        case PROP_CHEMICAL: return "Chemical";
        case PROP_ION: return "Ion";
        case PROP_FUSION: return "Fusion";
        case PROP_ANTIMATTER: return "Antimatter";
        case PROP_EXOTIC: return "Exotic";
        default: return "???";
    }
}

double ship_travel_fuel_cost(const Ship *s, double distance_au) {
    double eff;
    switch (s->propulsion) {
        case PROP_CHEMICAL: eff = 1.0; break;
        case PROP_ION: eff = 0.35; break;
        case PROP_FUSION: eff = 0.08; break;
        case PROP_ANTIMATTER: eff = 0.01; break;
        case PROP_EXOTIC: eff = 0.001; break;
        default: eff = 1.0;
    }
    return distance_au * eff * 10.0 / pow(1.2, s->engine_level - 1);
}

double ship_travel_time_seconds(const Ship *s, double distance_au) {
    double speed_factor;
    switch (s->propulsion) {
        case PROP_CHEMICAL: speed_factor = 1.0; break;
        case PROP_ION: speed_factor = 4.0; break;
        case PROP_FUSION: speed_factor = 20.0; break;
        case PROP_ANTIMATTER: speed_factor = 100.0; break;
        case PROP_EXOTIC: speed_factor = 500.0; break;
        default: speed_factor = 1.0;
    }
    double base_seconds_per_au = 5.0;
    return (distance_au * base_seconds_per_au) / (speed_factor * pow(1.15, s->engine_level - 1));
}
