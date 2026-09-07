#include "resources.h"
#include <string.h>

static const char *g_names[RES_COUNT] = {
    "Iron Ore", "Refined Iron", "Steel", "Machine Components", "Mining Equipment",
    "Silicon", "Carbon", "Water", "Aluminum", "Titanium", "Rare Metals",
    "Helium-3", "Hydrocarbons",
    "Exotic Matter", "Unknown Material"
};

static const char *g_symbols[RES_COUNT] = {
    "FeOre", "FeRf", "Steel", "MComp", "MinEq",
    "Si", "C", "H2O", "Al", "Ti", "RareM",
    "He3", "HC",
    "ExM", "???"
};

const char *resource_name(ResourceType t) {
    if (t < 0 || t >= RES_COUNT) return "Unknown";
    return g_names[t];
}

const char *resource_symbol(ResourceType t) {
    if (t < 0 || t >= RES_COUNT) return "??";
    return g_symbols[t];
}

void resource_store_init(ResourceStore *rs) {
    int i;
    for (i = 0; i < RES_COUNT; i++) rs->amount[i] = bn_zero();
}

int resource_can_afford(const ResourceStore *rs, const BigNum cost[RES_COUNT]) {
    int i;
    for (i = 0; i < RES_COUNT; i++) {
        if (cost[i].mantissa != 0 && !bn_gte(rs->amount[i], cost[i])) return 0;
    }
    return 1;
}

void resource_spend(ResourceStore *rs, const BigNum cost[RES_COUNT]) {
    int i;
    for (i = 0; i < RES_COUNT; i++) {
        if (cost[i].mantissa != 0) rs->amount[i] = bn_sub(rs->amount[i], cost[i]);
    }
}

void resource_add(ResourceStore *rs, ResourceType t, BigNum amount) {
    rs->amount[t] = bn_add(rs->amount[t], amount);
}
