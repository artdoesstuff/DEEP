#include "cli.h"
#include "simulation.h"
#include "save.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <time.h>

#define MAX_TOKENS 8
#define LINE_LEN 256

typedef enum {
    UPG_HULL = 0, UPG_ENGINE, UPG_CARGO, UPG_MINING, UPG_SCANNER, UPG_PROPULSION
} UpgradeSystem;

static void format_commas(double value, char *buf, size_t buflen) {
    long long v = (long long)(value + 0.5);
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%lld", v);
    int len = (int)strlen(tmp);
    int commas = (len - 1) / 3;
    int outlen = len + commas;
    if ((size_t)(outlen + 1) > buflen) { snprintf(buf, buflen, "%s", tmp); return; }
    buf[outlen] = '\0';
    int ti = len - 1, oi = outlen - 1, cnt = 0;
    while (ti >= 0) {
        buf[oi--] = tmp[ti--];
        cnt++;
        if (cnt % 3 == 0 && ti >= 0) buf[oi--] = ',';
    }
}

static void format_duration(double seconds, char *buf, size_t buflen) {
    long total = (long)seconds;
    long h = total / 3600;
    long m = (total % 3600) / 60;
    snprintf(buf, buflen, "%ldh %ldm", h, m);
}

static double res_total(const ResourceStore *rs) {
    int i; double t = 0;
    for (i = 0; i < RES_COUNT; i++) t += bn_to_double(rs->amount[i]);
    return t;
}

static int global_max_layer(GameState *g) {
    int i, best = LAYER_SURFACE;
    for (i = 0; i < g->planets.count; i++) {
        Planet *p = &g->planets.planets[i];
        int l = (int)planet_layer_at_depth(p, p->max_depth_reached_m);
        if (l > best) best = l;
    }
    return best;
}

static const char *prestige_tier_name(PrestigeTier t) {
    switch (t) {
        case PRESTIGE_NONE: return "None";
        case PRESTIGE_PLANETARY_EVACUATION: return "Planetary Evacuation";
        case PRESTIGE_SYSTEM_MIGRATION: return "System Migration";
        case PRESTIGE_INTERSTELLAR_MIGRATION: return "Interstellar Migration";
        case PRESTIGE_GALACTIC_EXPANSION: return "Galactic Expansion";
        default: return "???";
    }
}

static int count_stations_in_system(GameState *g, int system_id, int min_level) {
    int i, c = 0;
    for (i = 0; i < g->planets.count; i++) {
        Planet *p = &g->planets.planets[i];
        if (p->system_id == system_id && p->station_level >= min_level) c++;
    }
    return c;
}

void cli_print_banner(void) {
    printf("=====================================================\n");
    printf("   D.E.E.P. -- Deep Exploration & Extraction Platform\n");
    printf("=====================================================\n");
    printf("Type 'help' for a list of terminal commands.\n\n");
}

static void print_help(void) {
    printf("Available commands:\n");
    printf("  status                    Operation overview\n");
    printf("  mine                      Perform one mining action\n");
    printf("  planet [name]             List planets, or inspect one\n");
    printf("  travel <name>             Travel to another planet\n");
    printf("  ship                      Ship systems readout\n");
    printf("  research [name]           List research, or begin a project\n");
    printf("  upgrade [system]          List upgrades, or upgrade a ship system\n");
    printf("                            (hull, engine, cargo, mining, scanner, propulsion, station)\n");
    printf("  resources                 Cargo hold inventory\n");
    printf("  resources refine <name> [amount]  Refine some or all of a resource up the chain\n");
    printf("  discoveries               Discovery log\n");
    printf("  prestige                  Prestige status and requirements\n");
    printf("  prestige confirm          Perform a Planetary Evacuation, if eligible\n");
    printf("  prestige advance          Advance to the next prestige tier, if eligible\n");
    printf("  save [file]               Save game\n");
    printf("  load [file]               Load game\n");
    printf("  help                      Show this help\n");
    printf("  quit                      Exit DEEP\n");
}

static void print_status(GameState *g) {
    Planet *p = game_current_planet(g);
    LayerType lt = planet_layer_at_depth(p, p->current_depth_m);
    char depthbuf[64];
    format_commas(p->current_depth_m, depthbuf, sizeof(depthbuf));

    printf("--------------------------------------------------\n");
    printf(" LOCATION:  %s (%s)\n", p->name, layer_name(lt));
    printf(" DEPTH:     %s m\n", depthbuf);
    printf(" TEMPERATURE: %.0f K      PRESSURE: %.3e Pa\n",
        p->layers[lt].base_temp_k, p->layers[lt].base_pressure_pa);
    if (lt >= LAYER_UNKNOWN) {
        printf("\n WARNING:\n   Core boundary not detected.\n\n");
        printf(" Material detected:\n   UNKNOWN\n\n");
        printf(" Density:\n   NEGATIVE\n\n");
    }
    printf("--------------------------------------------------\n");
    printf(" SHIP: Hull L%d  Engine L%d (%s)  Cargo L%d  Drill L%d  Scanner L%d\n",
        g->ship.hull_level, g->ship.engine_level, propulsion_name(g->ship.propulsion),
        g->ship.cargo_level, g->ship.mining_equipment_level, g->ship.scanner_level);
    printf(" FUEL: %.1f/%.1f   POWER: %.1f/%.1f   LIFE SUPPORT: %.0f%%   WEAR: %.0f%%\n",
        g->ship.fuel, g->ship.fuel_capacity, g->ship.power, g->ship.power_capacity,
        g->ship.life_support, g->ship.equipment_wear);
    if (p->station_level > 0) {
        printf(" STATION HERE: Level %d (working this world autonomously)\n", p->station_level);
    }
    printf("--------------------------------------------------\n");
    int i, rc = 0;
    for (i = 0; i < g->research.count; i++) if (g->research.nodes[i].completed) rc++;
    printf(" RESEARCH COMPLETED: %d/%d\n", rc, g->research.count);
    printf(" DISCOVERIES: %d/%d\n", g->discoveries.total_discovered, g->discoveries.count);
    char insightbuf[32];
    bn_to_string(g->prestige.insight, insightbuf, sizeof(insightbuf));
    printf(" INSIGHT: %s   PRESTIGE TIER: %s\n", insightbuf, prestige_tier_name(g->prestige.tier));
    int stations = 0;
    for (i = 0; i < g->planets.count; i++) if (g->planets.planets[i].station_level > 0) stations++;
    printf(" AUTOMATION TECH: %s   ACTIVE MINING STATIONS: %d\n",
        g->automation_unlocked ? "unlocked" : "not yet researched", stations);
    char lifetimebuf[64];
    format_commas(g->player.total_meters_mined, lifetimebuf, sizeof(lifetimebuf));
    printf(" LIFETIME METERS MINED: %s m\n", lifetimebuf);
    char playtimebuf[32];
    format_duration(g->player.playtime_seconds, playtimebuf, sizeof(playtimebuf));
    printf(" LIFETIME PLAYTIME: %s\n", playtimebuf);
    printf("--------------------------------------------------\n");
}

static void print_mine_result(GameState *g, MiningResult *r) {
    if (r->hazard_triggered == -1) {
        printf("ACCESS DENIED: hull and field systems insufficient for this depth.\n");
        printf("Deep-Core Stabilization research is required to proceed past the Deep Crust.\n");
        return;
    }
    Planet *p = game_current_planet(g);
    char depthbuf[64];
    format_commas(p->current_depth_m, depthbuf, sizeof(depthbuf));
    printf("Drilling... descended %.1f m. Current depth: %s m\n", r->depth_gained, depthbuf);
    int i, any = 0;
    for (i = 0; i < RES_COUNT; i++) {
        if (r->yields[i].mantissa != 0) {
            if (!any) { printf("Extracted:\n"); any = 1; }
            char buf[32];
            bn_to_string(r->yields[i], buf, sizeof(buf));
            printf("   %-20s +%s\n", resource_name((ResourceType)i), buf);
        }
    }
    if (!any) printf("Cargo hold is full -- nothing more could be stored.\n");
    if (r->efficiency < 0.99) {
        printf("Crew fatigue and equipment strain are cutting effectiveness to %.0f%%.\n", r->efficiency * 100.0);
    }
    if (g->ship.equipment_wear > 85.0) {
        printf("Equipment is badly worn (%.0f%%) and needs downtime to repair.\n", g->ship.equipment_wear);
    }
    if (r->hazard_triggered == 1) {
        printf("WARNING: hazard encountered. Life support at %.0f%%.\n", g->ship.life_support);
    }
}

static void print_planet_list(GameState *g) {
    int i;
    printf("SOLAR SYSTEM AND BEYOND:\n");
    for (i = 0; i < g->planets.count; i++) {
        Planet *p = &g->planets.planets[i];
        const char *marker = (p->id == g->player.current_planet_id) ? "*" : " ";
        if (p->unlocked) {
            printf(" %s %-14s %10.2f AU   [%s]\n", marker, p->name, p->distance_from_sun_au,
                p->discovered ? "surveyed" : "unsurveyed");
        } else {
            printf(" %s %-14s (requires propulsion tier %d)\n", marker, "??? [locked]", p->min_tech_tier_to_unlock);
        }
    }
}

static void print_planet_detail(Planet *p) {
    printf("--------------------------------------------------\n");
    printf(" %s\n", p->name);
    printf("%s\n", p->description);
    printf("--------------------------------------------------\n");
    char depthbuf[64], maxbuf[64];
    format_commas(p->current_depth_m, depthbuf, sizeof(depthbuf));
    format_commas(p->max_depth_reached_m, maxbuf, sizeof(maxbuf));
    printf(" Current depth: %s m   Max reached: %s m\n", depthbuf, maxbuf);
    if (p->station_level > 0) {
        printf(" Mining station: Level %d (operating autonomously)\n", p->station_level);
    } else {
        printf(" Mining station: none\n");
    }
    printf(" Geological layers:\n");
    int i;
    for (i = 0; i < NUM_LAYERS; i++) {
        double maxd = p->layers[i].max_depth_m;
        if (maxd < 0)
            printf("   %-16s from %.0f m\n", layer_name((LayerType)i), p->layers[i].min_depth_m);
        else
            printf("   %-16s %.0f m - %.0f m\n", layer_name((LayerType)i), p->layers[i].min_depth_m, maxd);
    }
    printf("--------------------------------------------------\n");
}

static void do_travel(GameState *g, const char *name) {
    Planet *dest = planet_find(&g->planets, name);
    if (!dest) { printf("Unknown destination '%s'.\n", name); return; }
    if (dest->id == g->player.current_planet_id) { printf("Already at %s.\n", dest->name); return; }
    if (!dest->unlocked || (int)g->ship.propulsion < dest->min_tech_tier_to_unlock) {
        printf("%s is beyond current propulsion range (requires tier %d, you have %d: %s).\n",
            dest->name, dest->min_tech_tier_to_unlock, (int)g->ship.propulsion, propulsion_name(g->ship.propulsion));
        return;
    }
    Planet *cur = game_current_planet(g);
    double distance = fabs(dest->distance_from_sun_au - cur->distance_from_sun_au);
    if (distance < 0.05) distance = 0.05;
    double fuel_cost = ship_travel_fuel_cost(&g->ship, distance);
    double travel_time = ship_travel_time_seconds(&g->ship, distance);
    if (g->ship.fuel < fuel_cost) {
        printf("Insufficient fuel. Need %.1f, have %.1f.\n", fuel_cost, g->ship.fuel);
        return;
    }
    printf("Departing %s for %s...\n", cur->name, dest->name);
    g->ship.fuel -= fuel_cost;
    simulation_apply_offline_progress(g, (long)travel_time);
    g->player.current_planet_id = dest->id;
    dest->discovered = 1;
    printf("Arrived at %s. Travel time: %.0f s. Fuel used: %.1f\n", dest->name, travel_time, fuel_cost);
    printf("%s\n", dest->description);
}

static void print_ship(GameState *g) {
    Ship *s = &g->ship;
    printf("--------------------------------------------------\n");
    printf(" SHIP SYSTEMS\n");
    printf("--------------------------------------------------\n");
    printf("  Hull:            Level %d (hazard & wear resistance x%.2f)\n",
        s->hull_level, 1.0 / (1.0 + 0.2 * (s->hull_level - 1)));
    printf("  Engine:          Level %d (%s propulsion)\n", s->engine_level, propulsion_name(s->propulsion));
    printf("  Cargo capacity:  %.0f units (Level %d)\n", ship_cargo_capacity(s), s->cargo_level);
    printf("  Mining equip.:   Level %d (power x%.2f)\n", s->mining_equipment_level, ship_mining_power(s));
    printf("  Scanner:         Level %d (power x%.2f)\n", s->scanner_level, ship_scan_power(s));
    printf("  Fuel:            %.1f / %.1f\n", s->fuel, s->fuel_capacity);
    printf("  Power:           %.1f / %.1f\n", s->power, s->power_capacity);
    printf("  Life support:    %.0f%%%s\n", s->life_support,
        s->life_support < 70.0 ? "  (degraded -- mining efficiency reduced, recovers slowly over time)" : "");
    printf("  Equipment wear:  %.0f%%%s\n", s->equipment_wear,
        s->equipment_wear > 30.0 ? "  (worn -- mining efficiency reduced, repairs slowly when idle)" : "");
    printf("--------------------------------------------------\n");
}

static void get_station_cost(const Planet *p, BigNum cost[RES_COUNT]) {
    int i;
    for (i = 0; i < RES_COUNT; i++) cost[i] = bn_zero();
    int lvl = p->station_level; /* first build is level 0 -> 1 */
    cost[RES_MACHINE_COMPONENTS] = bn_from_double(40 * pow(1.7, lvl));
    cost[RES_MINING_EQUIPMENT] = bn_from_double(10 * pow(1.7, lvl));
}

static void do_station_upgrade(GameState *g) {
    if (!research_has(&g->research, RESEARCH_PLANETARY_INFRASTRUCTURE)) {
        printf("Building a mining station requires Planetary Infrastructure research first.\n");
        return;
    }
    Planet *cp = game_current_planet(g);
    BigNum cost[RES_COUNT];
    get_station_cost(cp, cost);
    if (!resource_can_afford(&g->player.resources, cost)) {
        printf("Insufficient resources for a station upgrade on %s.\n", cp->name);
        return;
    }
    resource_spend(&g->player.resources, cost);
    cp->station_level++;
    if (cp->station_level == 1) {
        printf("Mining station established on %s. It will keep working this world even while you are away or elsewhere.\n", cp->name);
    } else {
        printf("Mining station on %s upgraded to level %d.\n", cp->name, cp->station_level);
    }
}

static void get_upgrade_cost(GameState *g, UpgradeSystem sys, BigNum cost[RES_COUNT]) {
    int i;
    for (i = 0; i < RES_COUNT; i++) cost[i] = bn_zero();
    int lvl;
    switch (sys) {
        case UPG_HULL:
            lvl = g->ship.hull_level;
            cost[RES_STEEL] = bn_from_double(20 * pow(1.6, lvl - 1));
            cost[RES_TITANIUM] = bn_from_double(10 * pow(1.6, lvl - 1));
            break;
        case UPG_ENGINE:
            lvl = g->ship.engine_level;
            cost[RES_TITANIUM] = bn_from_double(15 * pow(1.6, lvl - 1));
            cost[RES_MACHINE_COMPONENTS] = bn_from_double(10 * pow(1.6, lvl - 1));
            break;
        case UPG_CARGO:
            lvl = g->ship.cargo_level;
            cost[RES_ALUMINUM] = bn_from_double(20 * pow(1.6, lvl - 1));
            cost[RES_STEEL] = bn_from_double(10 * pow(1.6, lvl - 1));
            break;
        case UPG_MINING:
            lvl = g->ship.mining_equipment_level;
            cost[RES_MACHINE_COMPONENTS] = bn_from_double(15 * pow(1.6, lvl - 1));
            cost[RES_MINING_EQUIPMENT] = bn_from_double(5 * pow(1.6, lvl - 1));
            break;
        case UPG_SCANNER:
            lvl = g->ship.scanner_level;
            cost[RES_SILICON] = bn_from_double(20 * pow(1.6, lvl - 1));
            cost[RES_RARE_METALS] = bn_from_double(8 * pow(1.6, lvl - 1));
            break;
        case UPG_PROPULSION:
            switch (g->ship.propulsion) {
                case PROP_CHEMICAL:
                    cost[RES_TITANIUM] = bn_from_double(60);
                    cost[RES_MACHINE_COMPONENTS] = bn_from_double(40);
                    break;
                case PROP_ION:
                    cost[RES_HELIUM3] = bn_from_double(150);
                    cost[RES_TITANIUM] = bn_from_double(120);
                    break;
                case PROP_FUSION:
                    cost[RES_EXOTIC_MATTER] = bn_from_double(20);
                    cost[RES_MACHINE_COMPONENTS] = bn_from_double(200);
                    break;
                case PROP_ANTIMATTER:
                    cost[RES_EXOTIC_MATTER] = bn_from_double(100);
                    cost[RES_UNKNOWN_MATERIAL] = bn_from_double(150);
                    break;
                default:
                    break;
            }
            break;
    }
}

static void print_upgrade_list(GameState *g) {
    printf("--------------------------------------------------\n");
    printf(" SHIP UPGRADES\n");
    printf("--------------------------------------------------\n");
    const char *names[] = { "hull", "engine", "cargo", "mining", "scanner", "propulsion" };
    int i, j;
    for (i = 0; i < 6; i++) {
        if (i == UPG_PROPULSION && g->ship.propulsion == PROP_EXOTIC) {
            printf(" %-12s (already at maximum: Exotic)\n", names[i]);
            continue;
        }
        BigNum cost[RES_COUNT];
        get_upgrade_cost(g, (UpgradeSystem)i, cost);
        printf(" %-12s cost: ", names[i]);
        int any = 0;
        for (j = 0; j < RES_COUNT; j++) {
            if (cost[j].mantissa != 0) {
                char buf[32]; bn_to_string(cost[j], buf, sizeof(buf));
                printf("%s%s %s", any ? ", " : "", buf, resource_name((ResourceType)j));
                any = 1;
            }
        }
        printf("\n");
    }
    printf("--------------------------------------------------\n");
    printf(" PLANETARY INFRASTRUCTURE\n");
    printf("--------------------------------------------------\n");
    Planet *cp = game_current_planet(g);
    if (!research_has(&g->research, RESEARCH_PLANETARY_INFRASTRUCTURE)) {
        printf(" station       (requires Planetary Infrastructure research)\n");
    } else {
        BigNum scost[RES_COUNT];
        get_station_cost(cp, scost);
        printf(" station       %s: level %d -> %d, cost: ", cp->name, cp->station_level, cp->station_level + 1);
        int any = 0, j2;
        for (j2 = 0; j2 < RES_COUNT; j2++) {
            if (scost[j2].mantissa != 0) {
                char buf[32]; bn_to_string(scost[j2], buf, sizeof(buf));
                printf("%s%s %s", any ? ", " : "", buf, resource_name((ResourceType)j2));
                any = 1;
            }
        }
        printf("\n");
    }
    printf("--------------------------------------------------\n");
}

static void do_upgrade(GameState *g, const char *sys) {
    if (strcasecmp(sys, "station") == 0) { do_station_upgrade(g); return; }

    UpgradeSystem u;
    int *level_ptr = NULL;
    if (strcasecmp(sys, "hull") == 0) { u = UPG_HULL; level_ptr = &g->ship.hull_level; }
    else if (strcasecmp(sys, "engine") == 0) { u = UPG_ENGINE; level_ptr = &g->ship.engine_level; }
    else if (strcasecmp(sys, "cargo") == 0) { u = UPG_CARGO; level_ptr = &g->ship.cargo_level; }
    else if (strcasecmp(sys, "mining") == 0 || strcasecmp(sys, "drill") == 0) { u = UPG_MINING; level_ptr = &g->ship.mining_equipment_level; }
    else if (strcasecmp(sys, "scanner") == 0) { u = UPG_SCANNER; level_ptr = &g->ship.scanner_level; }
    else if (strcasecmp(sys, "propulsion") == 0) { u = UPG_PROPULSION; }
    else { printf("Unknown ship system '%s'. See 'upgrade' for options.\n", sys); return; }

    BigNum cost[RES_COUNT];
    get_upgrade_cost(g, u, cost);

    if (u == UPG_PROPULSION) {
        if (g->ship.propulsion == PROP_EXOTIC) { printf("Propulsion is already at maximum (Exotic).\n"); return; }
        PropulsionType next = (PropulsionType)(g->ship.propulsion + 1);
        ResearchId req;
        switch (g->ship.propulsion) {
            case PROP_CHEMICAL: req = RESEARCH_ION_PROPULSION; break;
            case PROP_ION: req = RESEARCH_FUSION_PROPULSION; break;
            case PROP_FUSION: req = RESEARCH_ANTIMATTER_CONTAINMENT; break;
            default: req = RESEARCH_DIMENSIONAL_SURVEYING; break;
        }
        if (!research_has(&g->research, req)) {
            printf("Propulsion upgrade requires further research first.\n");
            return;
        }
        if (!resource_can_afford(&g->player.resources, cost)) { printf("Insufficient resources.\n"); return; }
        resource_spend(&g->player.resources, cost);
        g->ship.propulsion = next;
        printf("Propulsion upgraded to %s drive.\n", propulsion_name(g->ship.propulsion));
        return;
    }

    if (!resource_can_afford(&g->player.resources, cost)) { printf("Insufficient resources for %s upgrade.\n", sys); return; }
    resource_spend(&g->player.resources, cost);
    (*level_ptr)++;
    printf("%s upgraded to level %d.\n", sys, *level_ptr);
}

static void print_resources(GameState *g) {
    printf("--------------------------------------------------\n");
    printf(" CARGO HOLD  (%.0f / %.0f units)\n", res_total(&g->player.resources), ship_cargo_capacity(&g->ship));
    printf("--------------------------------------------------\n");
    int i;
    for (i = 0; i < RES_COUNT; i++) {
        if (g->player.resources.amount[i].mantissa == 0) continue;
        char buf[32];
        bn_to_string(g->player.resources.amount[i], buf, sizeof(buf));
        printf("  %-20s %s\n", resource_name((ResourceType)i), buf);
    }
    printf("--------------------------------------------------\n");
}

static int refine_target(ResourceType from, ResourceType *out_to, double *ratio) {
    switch (from) {
        case RES_IRON_ORE: *out_to = RES_REFINED_IRON; *ratio = 3.0; return 1;
        case RES_REFINED_IRON: *out_to = RES_STEEL; *ratio = 2.0; return 1;
        case RES_STEEL: *out_to = RES_MACHINE_COMPONENTS; *ratio = 3.0; return 1;
        case RES_MACHINE_COMPONENTS: *out_to = RES_MINING_EQUIPMENT; *ratio = 3.0; return 1;
        default: return 0;
    }
}

static void do_refine(GameState *g, const char *name, double requested_amount) {
    int i;
    int found = -1;
    for (i = 0; i < RES_COUNT; i++) {
        if (strcasecmp(resource_name((ResourceType)i), name) == 0) { found = i; break; }
    }
    if (found == -1) {
        for (i = 0; i < RES_COUNT; i++) {
            if (strncasecmp(resource_name((ResourceType)i), name, strlen(name)) == 0) { found = i; break; }
        }
    }
    if (found == -1) { printf("Unknown resource '%s'.\n", name); return; }

    ResourceType to; double ratio;
    if (!refine_target((ResourceType)found, &to, &ratio)) {
        printf("%s cannot be refined any further.\n", resource_name((ResourceType)found));
        return;
    }
    double have = bn_to_double(g->player.resources.amount[found]);
    double usable = have;
    if (requested_amount > 0.0 && requested_amount < usable) usable = requested_amount;
    if (usable < ratio - 1e-6) {
        printf("Insufficient %s to refine (need %.0f, have %.1f).\n", resource_name((ResourceType)found), ratio, have);
        return;
    }
    double batches = floor(usable / ratio + 1e-9);
    double consume = batches * ratio;
    g->player.resources.amount[found] = bn_sub(g->player.resources.amount[found], bn_from_double(consume));
    resource_add(&g->player.resources, to, bn_from_double(batches));
    printf("Refined %.0f %s into %.0f %s.\n", consume, resource_name((ResourceType)found), batches, resource_name(to));
}

static void print_research_list(GameState *g) {
    printf("--------------------------------------------------\n");
    printf(" RESEARCH PROJECTS\n");
    printf("--------------------------------------------------\n");
    int i, j;
    for (i = 0; i < g->research.count; i++) {
        ResearchNode *n = &g->research.nodes[i];
        const char *state;
        if (n->completed) state = "[DONE]";
        else if (!research_is_unlocked_prereqs(&g->research, n)) state = "[LOCKED]";
        else if (n->min_depth_layer >= 0 && global_max_layer(g) < n->min_depth_layer) state = "[LOCKED-DEPTH]";
        else state = "[AVAILABLE]";
        printf(" %-14s %-26s %s\n", state, n->name, n->description);
        if (strcmp(state, "[AVAILABLE]") == 0) {
            printf("                Cost: ");
            int any = 0;
            for (j = 0; j < RES_COUNT; j++) {
                if (n->cost[j].mantissa != 0) {
                    char buf[32]; bn_to_string(n->cost[j], buf, sizeof(buf));
                    printf("%s%s %s", any ? ", " : "", buf, resource_name((ResourceType)j));
                    any = 1;
                }
            }
            printf("\n");
        }
    }
    printf("--------------------------------------------------\n");
}

static void do_research(GameState *g, const char *name) {
    ResearchNode *n = research_find(&g->research, name);
    if (!n) { printf("Unknown research project '%s'.\n", name); return; }
    if (n->completed) { printf("%s is already complete.\n", n->name); return; }
    if (!research_is_unlocked_prereqs(&g->research, n)) { printf("%s requires prerequisite research first.\n", n->name); return; }
    if (n->min_depth_layer >= 0 && global_max_layer(g) < n->min_depth_layer) {
        printf("%s requires reaching the %s layer on some planet first.\n", n->name, layer_name((LayerType)n->min_depth_layer));
        return;
    }
    if (!resource_can_afford(&g->player.resources, n->cost)) {
        printf("Insufficient resources for %s.\n", n->name);
        return;
    }
    resource_spend(&g->player.resources, n->cost);
    n->completed = 1;
    printf("Research complete: %s\n", n->name);
    printf("%s\n", n->description);
    if (n->id == RESEARCH_AUTONOMOUS_MINING) {
        g->automation_unlocked = 1;
        printf("Automated drone technology online. Planetary Infrastructure research will let you build stations.\n");
    } else if (n->id == RESEARCH_PLANETARY_INFRASTRUCTURE) {
        printf("You can now build a mining station on any planet with 'upgrade station'.\n");
        printf("A station keeps working that specific world autonomously, even while your ship is elsewhere.\n");
    } else if (n->id == RESEARCH_ORBITAL_LOGISTICS) {
        printf("Cargo capacity increased. Station output is now delivered directly to your stockpile.\n");
    } else if (n->id == RESEARCH_STAR_SYSTEM_INDUSTRY) {
        printf("All mining stations now operate as a coordinated industrial network.\n");
    } else if (n->id == RESEARCH_ATMOSPHERIC_EXTRACTION) {
        printf("Extraction yields are now boosted on any world with a harvestable atmosphere.\n");
    } else if (n->id == RESEARCH_QUANTUM_SCANNING) {
        printf("Discovery odds are now sharply increased everywhere you operate.\n");
    } else if (n->id == RESEARCH_DIMENSIONAL_SURVEYING) {
        Discovery *pattern = discovery_find(&g->discoveries, "The Pattern Emerges");
        if (pattern && !pattern->discovered) {
            pattern->discovered = 1;
            g->discoveries.total_discovered++;
            printf("\n*** DISCOVERY: %s ***\n%s\n", pattern->name, pattern->text);
        }
    }
}

static void print_discoveries(GameState *g) {
    printf("--------------------------------------------------\n");
    printf(" DISCOVERY LOG (%d/%d)\n", g->discoveries.total_discovered, g->discoveries.count);
    printf("--------------------------------------------------\n");
    int i;
    for (i = 0; i < g->discoveries.count; i++) {
        Discovery *d = &g->discoveries.items[i];
        if (d->discovered) {
            printf(" [%s]\n   %s\n\n", d->name, d->text);
        } else {
            printf(" [??? undiscovered]\n\n");
        }
    }
    printf("--------------------------------------------------\n");
}

#define EVACUATION_THRESHOLD_M 50000.0

static void print_prestige(GameState *g) {
    Planet *p = game_current_planet(g);
    printf("--------------------------------------------------\n");
    printf(" PRESTIGE STATUS\n");
    printf("--------------------------------------------------\n");
    printf("  Highest tier reached: %s\n", prestige_tier_name(g->prestige.tier));
    printf("  Evacuations performed: %d\n", g->prestige.evacuation_count);
    char buf[32]; bn_to_string(g->prestige.insight, buf, sizeof(buf));
    printf("  Insight: %s\n", buf);
    printf("  Permanent mining bonus: x%.2f\n", g->prestige.mining_bonus_mult);
    printf("--------------------------------------------------\n");

    if (p->max_depth_reached_m >= EVACUATION_THRESHOLD_M) {
        int rc = 0, i;
        for (i = 0; i < g->research.count; i++) if (g->research.nodes[i].completed) rc++;
        BigNum gain = prestige_calc_insight_gain(p->max_depth_reached_m, rc);
        char gbuf[32]; bn_to_string(gain, gbuf, sizeof(gbuf));
        printf("  Planetary Evacuation available on %s. Estimated insight gain: %s\n", p->name, gbuf);
        printf("  Type 'prestige confirm' to abandon this operation and evacuate.\n");
    } else {
        printf("  Planetary Evacuation requires reaching Deep Crust (50,000 m) on your current planet.\n");
    }

    printf("--------------------------------------------------\n");
    switch (g->prestige.tier) {
        case PRESTIGE_NONE:
            printf("  Next milestone: System Migration -- perform a Planetary Evacuation first.\n");
            break;
        case PRESTIGE_PLANETARY_EVACUATION: {
            int stations = count_stations_in_system(g, 0, 1);
            printf("  Next milestone: System Migration (\"I operate a star system.\")\n");
            printf("    Requires active mining stations on 3+ Sol planets: %d/3\n", stations);
            printf("    Type 'prestige advance' once ready.\n");
            break;
        }
        case PRESTIGE_SYSTEM_MIGRATION: {
            int stations = count_stations_in_system(g, 1, 1);
            printf("  Next milestone: Interstellar Migration (\"I control an interstellar industrial network.\")\n");
            printf("    Requires Antimatter propulsion (have: %s) and a station in the Proxima system: %d/1\n",
                propulsion_name(g->ship.propulsion), stations);
            printf("    Type 'prestige advance' once ready.\n");
            break;
        }
        case PRESTIGE_INTERSTELLAR_MIGRATION: {
            int stations = count_stations_in_system(g, 2, 1);
            printf("  Next milestone: Galactic Expansion (\"I am reshaping a galaxy.\")\n");
            printf("    Requires Exotic propulsion (have: %s), a station at Kepler-186f: %d/1,\n",
                propulsion_name(g->ship.propulsion), stations);
            printf("    and having reached Impossible Depth somewhere: %s\n",
                global_max_layer(g) >= LAYER_IMPOSSIBLE ? "yes" : "no");
            printf("    Type 'prestige advance' once ready.\n");
            break;
        }
        default:
            printf("  You have reached the highest known prestige tier.\n");
            break;
    }
    printf("--------------------------------------------------\n");
}

static void do_prestige_confirm(GameState *g) {
    Planet *p = game_current_planet(g);
    if (p->max_depth_reached_m < EVACUATION_THRESHOLD_M) {
        printf("Evacuation criteria not met.\n");
        return;
    }
    int rc = 0, i;
    for (i = 0; i < g->research.count; i++) if (g->research.nodes[i].completed) rc++;
    BigNum gain = prestige_calc_insight_gain(p->max_depth_reached_m, rc);
    g->prestige.insight = bn_add(g->prestige.insight, gain);
    g->prestige.evacuation_count++;
    if (g->prestige.tier < PRESTIGE_PLANETARY_EVACUATION) g->prestige.tier = PRESTIGE_PLANETARY_EVACUATION;
    g->prestige.mining_bonus_mult += bn_to_double(gain) * 0.01;

    resource_store_init(&g->player.resources);
    g->player.resources.amount[RES_IRON_ORE] = bn_from_double(20);
    p->current_depth_m = 0;

    char buf[32]; bn_to_string(gain, buf, sizeof(buf));
    printf("PLANETARY EVACUATION COMPLETE.\n");
    printf("Local infrastructure and stockpiles on %s have been abandoned.\n", p->name);
    printf("Insight gained: %s\n", buf);
    printf("Permanent mining bonus is now x%.2f\n", g->prestige.mining_bonus_mult);
}

static void do_prestige_advance(GameState *g) {
    switch (g->prestige.tier) {
        case PRESTIGE_NONE:
            printf("Perform a Planetary Evacuation first ('prestige confirm').\n");
            break;

        case PRESTIGE_PLANETARY_EVACUATION: {
            int stations = count_stations_in_system(g, 0, 1);
            if (stations < 3) {
                printf("System Migration requires active mining stations on at least 3 Sol planets (currently %d).\n", stations);
                return;
            }
            g->prestige.tier = PRESTIGE_SYSTEM_MIGRATION;
            g->prestige.mining_bonus_mult *= 1.25;
            printf("SYSTEM MIGRATION COMPLETE.\n");
            printf("Your operation now spans the Sol system.\n");
            printf("\"I operate a star system.\"\n");
            printf("Permanent mining bonus is now x%.2f\n", g->prestige.mining_bonus_mult);
            break;
        }

        case PRESTIGE_SYSTEM_MIGRATION: {
            if (g->ship.propulsion < PROP_ANTIMATTER) {
                printf("Interstellar Migration requires Antimatter propulsion.\n");
                return;
            }
            int stations = count_stations_in_system(g, 1, 1);
            if (stations < 1) {
                printf("Interstellar Migration requires an active mining station in the Proxima system.\n");
                return;
            }
            g->prestige.tier = PRESTIGE_INTERSTELLAR_MIGRATION;
            g->prestige.mining_bonus_mult *= 1.5;
            printf("INTERSTELLAR MIGRATION COMPLETE.\n");
            printf("Your industry now spans star systems.\n");
            printf("\"I control an interstellar industrial network.\"\n");
            printf("Permanent mining bonus is now x%.2f\n", g->prestige.mining_bonus_mult);
            break;
        }

        case PRESTIGE_INTERSTELLAR_MIGRATION: {
            if (g->ship.propulsion < PROP_EXOTIC) {
                printf("Galactic Expansion requires Exotic propulsion.\n");
                return;
            }
            int stations = count_stations_in_system(g, 2, 1);
            if (stations < 1) {
                printf("Galactic Expansion requires an active mining station at Kepler-186f.\n");
                return;
            }
            if (global_max_layer(g) < LAYER_IMPOSSIBLE) {
                printf("Galactic Expansion requires having reached Impossible Depth somewhere first.\n");
                return;
            }
            g->prestige.tier = PRESTIGE_GALACTIC_EXPANSION;
            g->prestige.mining_bonus_mult *= 2.0;
            printf("GALACTIC EXPANSION COMPLETE.\n");
            printf("\"I am reshaping a galaxy.\"\n");
            printf("\n");
            printf("You understand now, at least in part, what you have actually been digging\n");
            printf("through all this time. It was never just rock.\n");
            printf("Permanent mining bonus is now x%.2f\n", g->prestige.mining_bonus_mult);
            break;
        }

        default:
            printf("You have already reached the highest known prestige tier.\n");
            break;
    }
}

void cli_run(GameState *g) {
    char line[LINE_LEN];
    char *tokens[MAX_TOKENS];
    cli_print_banner();

    while (g->running) {
        simulation_tick_real_time(g);
        printf("deep> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';

        int ntok = 0;
        char *tok = strtok(line, " ");
        while (tok && ntok < MAX_TOKENS) { tokens[ntok++] = tok; tok = strtok(NULL, " "); }
        if (ntok == 0) continue;

        if (strcasecmp(tokens[0], "status") == 0) {
            print_status(g);

        } else if (strcasecmp(tokens[0], "mine") == 0) {
            MiningResult r = simulation_do_manual_mine(g);
            print_mine_result(g, &r);
            if (r.hazard_triggered != -1) {
                Planet *cp = game_current_planet(g);
                int layer = (int)planet_layer_at_depth(cp, cp->current_depth_m);
                double scanner_power = ship_scan_power(&g->ship)
                    * (research_has(&g->research, RESEARCH_QUANTUM_SCANNING) ? 2.0 : 1.0);
                Discovery *d = discovery_try_roll(&g->discoveries, cp->id, layer, scanner_power);
                if (d) printf("\n*** DISCOVERY: %s ***\n%s\n", d->name, d->text);
            }

        } else if (strcasecmp(tokens[0], "planet") == 0) {
            if (ntok >= 2) {
                Planet *p = planet_find(&g->planets, tokens[1]);
                if (p) print_planet_detail(p); else printf("Unknown planet '%s'.\n", tokens[1]);
            } else {
                print_planet_list(g);
            }

        } else if (strcasecmp(tokens[0], "travel") == 0) {
            if (ntok >= 2) do_travel(g, tokens[1]);
            else printf("Usage: travel <planet name>\n");

        } else if (strcasecmp(tokens[0], "ship") == 0) {
            print_ship(g);

        } else if (strcasecmp(tokens[0], "research") == 0) {
            if (ntok >= 2) {
                char namebuf[128] = "";
                int i;
                for (i = 1; i < ntok; i++) { strcat(namebuf, tokens[i]); if (i < ntok - 1) strcat(namebuf, " "); }
                do_research(g, namebuf);
            } else {
                print_research_list(g);
            }

        } else if (strcasecmp(tokens[0], "upgrade") == 0) {
            if (ntok >= 2) do_upgrade(g, tokens[1]);
            else print_upgrade_list(g);

        } else if (strcasecmp(tokens[0], "resources") == 0) {
            if (ntok >= 3 && strcasecmp(tokens[1], "refine") == 0) {
                double amount = 0.0;
                int name_end = ntok;
                char *endptr = NULL;
                double val = strtod(tokens[ntok - 1], &endptr);
                if (endptr != tokens[ntok - 1] && *endptr == '\0' && ntok - 2 >= 1) {
                    amount = val;
                    name_end = ntok - 1;
                }
                char namebuf[128] = "";
                int i;
                for (i = 2; i < name_end; i++) { strcat(namebuf, tokens[i]); if (i < name_end - 1) strcat(namebuf, " "); }
                do_refine(g, namebuf, amount);
            } else {
                print_resources(g);
            }

        } else if (strcasecmp(tokens[0], "discoveries") == 0) {
            print_discoveries(g);

        } else if (strcasecmp(tokens[0], "prestige") == 0) {
            if (ntok >= 2 && strcasecmp(tokens[1], "confirm") == 0) do_prestige_confirm(g);
            else if (ntok >= 2 && strcasecmp(tokens[1], "advance") == 0) do_prestige_advance(g);
            else print_prestige(g);

        } else if (strcasecmp(tokens[0], "save") == 0) {
            const char *path = ntok >= 2 ? tokens[1] : "deep_save.dat";
            if (save_game(g, path) == 0) printf("Game saved to %s\n", path);
            else printf("Failed to save game.\n");

        } else if (strcasecmp(tokens[0], "load") == 0) {
            const char *path = ntok >= 2 ? tokens[1] : "deep_save.dat";
            time_t prev_tick = g->last_tick_time;
            int rc = load_game(g, path);
            if (rc == 0) {
                long elapsed = (long)difftime(time(NULL), prev_tick);
                printf("Game loaded from %s\n", path);
                if (elapsed > 5) {
                    simulation_apply_offline_progress(g, elapsed);
                    printf("Time elapsed since last save: %ld seconds. Any active stations kept working.\n", elapsed);
                }
            } else {
                printf("Failed to load game from %s (code %d)\n", path, rc);
            }

        } else if (strcasecmp(tokens[0], "help") == 0) {
            print_help();

        } else if (strcasecmp(tokens[0], "quit") == 0 || strcasecmp(tokens[0], "exit") == 0) {
            printf("Autosaving before exit...\n");
            save_game(g, "deep_save.dat");
            g->running = 0;

        } else {
            printf("Unknown command '%s'. Type 'help' for commands.\n", tokens[0]);
        }
    }
    printf("Connection to DEEP terminal closed.\n");
}
