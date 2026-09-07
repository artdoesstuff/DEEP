#include "simulation.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define BASE_DEPTH_PER_ACTION 4.0
#define BASE_YIELD_PER_ACTION 5.0
#define BASE_ENERGY_PER_ACTION 2.0

static double layer_weight_sum(const PlanetLayer *l) {
    int i;
    double s = 0;
    for (i = 0; i < RES_COUNT; i++) s += l->resource_weight[i];
    return s;
}

static void distribute_yield(const PlanetLayer *l, double total_yield, BigNum out[RES_COUNT]) {
    double sum = layer_weight_sum(l);
    int i;
    for (i = 0; i < RES_COUNT; i++) out[i] = bn_zero();
    if (sum <= 0.0) return;
    for (i = 0; i < RES_COUNT; i++) {
        if (l->resource_weight[i] <= 0.0) continue;
        double portion = total_yield * (l->resource_weight[i] / sum);
        out[i] = bn_from_double(portion);
    }
}

static double research_speed_bonus(const ResearchState *rs) {
    double bonus = 1.0;
    if (research_has(rs, RESEARCH_ADVANCED_DRILLING)) bonus *= 1.5;
    if (research_has(rs, RESEARCH_DEEP_CORE_STABILIZATION)) bonus *= 1.3;
    return bonus;
}

/* Crew fatigue and equipment strain: mining is worse when
 * life support is degraded */
static double life_support_penalty(double life_support) {
    if (life_support >= 70.0) return 1.0;
    if (life_support >= 40.0) return 0.8;
    if (life_support >= 15.0) return 0.55;
    return 0.3;
}

/* Harsh environments (Venus's heat and pressure above all) wear down
 * mining equipment over time. Worn equipment mines slower until it's
 * repaired, which happens passively but slowly */
static double equipment_wear_penalty(double wear) {
    if (wear <= 30.0) return 1.0;
    if (wear <= 60.0) return 0.85;
    if (wear <= 85.0) return 0.65;
    return 0.4;
}

/* A stronger hull is structural protection: it doesn't change how fast you
 * mine, but it makes hazards less likely to occur at all and blunts how
 * hard harsh environments chew through equipment */
static double hull_protection_factor(int hull_level) {
    return 1.0 / (1.0 + 0.2 * (hull_level - 1));
}

/* Atmospheric Extraction pays off specifically on worlds with a real
 * atmosphere to harvest from - anywhere hydrocarbons show up in the
 * local resource mix */
static double atmospheric_bonus(GameState *g, const PlanetLayer *layer) {
    if (!research_has(&g->research, RESEARCH_ATMOSPHERIC_EXTRACTION)) return 1.0;
    if (layer->resource_weight[RES_HYDROCARBONS] > 0.0) return 1.3;
    return 1.0;
}

/* Ship systems recover slowly and passively over real time: fuel
 * synthesises, capacitors trickle-charge, and basic repair systems patch
 * up life support - doesn't mean that you can out-recover hazards */
static void recharge_ship(Ship *s, double seconds) {
    if (seconds <= 0.0) return;
    s->fuel += s->fuel_capacity * 0.004 * seconds;
    if (s->fuel > s->fuel_capacity) s->fuel = s->fuel_capacity;
    s->power += s->power_capacity * 0.01 * seconds;
    if (s->power > s->power_capacity) s->power = s->power_capacity;
    s->life_support += 0.02 * seconds;
    if (s->life_support > 100.0) s->life_support = 100.0;
    s->equipment_wear -= 0.015 * seconds;
    if (s->equipment_wear < 0.0) s->equipment_wear = 0.0;
}

static double cargo_used(const ResourceStore *rs) {
    int i;
    double total = 0;
    for (i = 0; i < RES_COUNT; i++) total += bn_to_double(rs->amount[i]);
    return total;
}

static double cargo_capacity_total(GameState *g) {
    double cap = ship_cargo_capacity(&g->ship);
    if (research_has(&g->research, RESEARCH_ORBITAL_LOGISTICS)) cap *= 1.5;
    return cap;
}

static void deposit_with_cargo_cap(GameState *g, const BigNum raw_yield[RES_COUNT], BigNum out[RES_COUNT]) {
    double cap = cargo_capacity_total(g);
    double used = cargo_used(&g->player.resources);
    int i;
    for (i = 0; i < RES_COUNT; i++) {
        double amt = bn_to_double(raw_yield[i]);
        if (amt <= 0) { if (out) out[i] = bn_zero(); continue; }
        if (used + amt > cap) {
            amt = cap - used;
            if (amt < 0) amt = 0;
        }
        used += amt;
        BigNum add = bn_from_double(amt);
        resource_add(&g->player.resources, (ResourceType)i, add);
        if (out) out[i] = add;
    }
}

MiningResult simulation_do_manual_mine(GameState *g) {
    MiningResult res;
    memset(&res, 0, sizeof(res));

    Planet *planet = game_current_planet(g);
    LayerType layer_t = planet_layer_at_depth(planet, planet->current_depth_m);
    const PlanetLayer *layer = &planet->layers[layer_t];

    if (layer_t >= LAYER_MANTLE && !research_has(&g->research, RESEARCH_DEEP_CORE_STABILIZATION)) {
        res.hazard_triggered = -1;
        return res;
    }

    double mining_power = ship_mining_power(&g->ship) * layer->mining_speed_mult
                         * research_speed_bonus(&g->research) * g->prestige.mining_bonus_mult
                         * life_support_penalty(g->ship.life_support)
                         * equipment_wear_penalty(g->ship.equipment_wear);
    double depth_gain = BASE_DEPTH_PER_ACTION * mining_power;
    double yield_total = BASE_YIELD_PER_ACTION * mining_power * atmospheric_bonus(g, layer);
    double energy_needed = BASE_ENERGY_PER_ACTION * layer->energy_cost_mult;

    if (g->ship.power < energy_needed) {
        depth_gain *= 0.25;
        yield_total *= 0.25;
    } else {
        g->ship.power -= energy_needed;
    }
    recharge_ship(&g->ship, 3.0); /* one manual mining action represents a few seconds of activity */

    /* Equipment wears down faster in harsher environments - this is what
     * makes Venus specifically brutal on gear, not just slow to mine; however,
     * a stronger hull blunts the rate */
    double hull_factor = hull_protection_factor(g->ship.hull_level);
    g->ship.equipment_wear += 1.2 * layer->energy_cost_mult * hull_factor;
    if (g->ship.equipment_wear > 100.0) g->ship.equipment_wear = 100.0;

    BigNum raw_yield[RES_COUNT];
    distribute_yield(layer, yield_total, raw_yield);
    deposit_with_cargo_cap(g, raw_yield, res.yields);

    planet->current_depth_m += depth_gain;
    if (planet->current_depth_m > planet->max_depth_reached_m)
        planet->max_depth_reached_m = planet->current_depth_m;
    g->player.total_meters_mined += depth_gain;
    res.depth_gained = depth_gain;
    res.efficiency = life_support_penalty(g->ship.life_support);

    double hz = (double)rand() / (double)RAND_MAX;
    double effective_hazard_chance = layer->hazard_chance * hull_factor;
    if (hz < effective_hazard_chance) {
        res.hazard_triggered = 1;
        g->ship.life_support -= 2.0 + layer->hazard_chance * 10.0;
        if (g->ship.life_support < 0) g->ship.life_support = 0;
    }

    return res;
}

/* Mining stations are permanent planetary infrastructure: once built, they
 * keep working that specific planet's own depth/layer regardless of where
 * the ship currently is */
static double station_mining_power(GameState *g, const Planet *p) {
    if (p->station_level <= 0) return 0.0;
    double industry = research_has(&g->research, RESEARCH_STAR_SYSTEM_INDUSTRY) ? 1.5 : 1.0;
    return pow(1.4, p->station_level - 1) * research_speed_bonus(&g->research)
         * industry * g->prestige.mining_bonus_mult;
}

static void tick_all_stations(GameState *g, double seconds) {
    if (seconds <= 0.0) return;
    int i;
    for (i = 0; i < g->planets.count; i++) {
        Planet *p = &g->planets.planets[i];
        if (p->station_level <= 0) continue;

        LayerType layer_t = planet_layer_at_depth(p, p->current_depth_m);
        if (layer_t >= LAYER_MANTLE && !research_has(&g->research, RESEARCH_DEEP_CORE_STABILIZATION)) {
            continue; /* station can't push past the same physical gate a ship can't */
        }
        const PlanetLayer *layer = &p->layers[layer_t];

        double power = station_mining_power(g, p) * layer->mining_speed_mult;
        if (power <= 0.0) continue;

        /* Stations are steady but slower than a hands-on manual pass. */
        double rate_depth = (BASE_DEPTH_PER_ACTION * power * 0.5) / 3.0;
        double rate_yield = (BASE_YIELD_PER_ACTION * power * 0.5) / 3.0 * atmospheric_bonus(g, layer);

        double total_depth = rate_depth * seconds;
        double total_yield = rate_yield * seconds;

        BigNum raw_yield[RES_COUNT];
        distribute_yield(layer, total_yield, raw_yield);
        deposit_with_cargo_cap(g, raw_yield, NULL);

        p->current_depth_m += total_depth;
        if (p->current_depth_m > p->max_depth_reached_m) p->max_depth_reached_m = p->current_depth_m;
        g->player.total_meters_mined += total_depth;
    }
}

void simulation_apply_offline_progress(GameState *g, long elapsed_seconds) {
    if (elapsed_seconds <= 0) return;
    long cap_seconds = 8L * 3600L;
    long secs = elapsed_seconds > cap_seconds ? cap_seconds : elapsed_seconds;
    tick_all_stations(g, (double)secs);
    recharge_ship(&g->ship, (double)secs);
    g->player.playtime_seconds += (double)secs;
}

void simulation_tick_real_time(GameState *g) {
    time_t now = time(NULL);
    double elapsed = difftime(now, g->last_tick_time);
    if (elapsed > 0.0) {
        tick_all_stations(g, elapsed);
        recharge_ship(&g->ship, elapsed);
        g->player.playtime_seconds += elapsed;
    }
    g->last_tick_time = now;
}
