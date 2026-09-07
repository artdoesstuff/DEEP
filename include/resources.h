#ifndef DEEP_RESOURCES_H
#define DEEP_RESOURCES_H

#include "bignum.h"

typedef enum {
    RES_IRON_ORE = 0,
    RES_REFINED_IRON,
    RES_STEEL,
    RES_MACHINE_COMPONENTS,
    RES_MINING_EQUIPMENT,
    RES_SILICON,
    RES_CARBON,
    RES_WATER,
    RES_ALUMINUM,
    RES_TITANIUM,
    RES_RARE_METALS,
    RES_HELIUM3,
    RES_HYDROCARBONS,
    RES_EXOTIC_MATTER,
    RES_UNKNOWN_MATERIAL,
    RES_COUNT
} ResourceType;

typedef struct {
    BigNum amount[RES_COUNT];
} ResourceStore;

const char *resource_name(ResourceType t);
const char *resource_symbol(ResourceType t);

void resource_store_init(ResourceStore *rs);
int resource_can_afford(const ResourceStore *rs, const BigNum cost[RES_COUNT]);
void resource_spend(ResourceStore *rs, const BigNum cost[RES_COUNT]);
void resource_add(ResourceStore *rs, ResourceType t, BigNum amount);

#endif
