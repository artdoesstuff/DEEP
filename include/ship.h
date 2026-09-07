#ifndef DEEP_SHIP_H
#define DEEP_SHIP_H

typedef enum {
    PROP_CHEMICAL = 0,
    PROP_ION,
    PROP_FUSION,
    PROP_ANTIMATTER,
    PROP_EXOTIC
} PropulsionType;

typedef struct {
    int hull_level;
    int engine_level;
    int cargo_level;
    int mining_equipment_level;
    int scanner_level;
    double fuel;
    double fuel_capacity;
    double power;
    double power_capacity;
    double life_support;   /* 0-100 percent */
    double equipment_wear; /* 0 = pristine, 100 = fully degraded */
    PropulsionType propulsion;
} Ship;

void ship_init_default(Ship *s);
double ship_cargo_capacity(const Ship *s);
double ship_mining_power(const Ship *s);
double ship_scan_power(const Ship *s);
const char *propulsion_name(PropulsionType p);
double ship_travel_fuel_cost(const Ship *s, double distance_au);
double ship_travel_time_seconds(const Ship *s, double distance_au);

#endif
