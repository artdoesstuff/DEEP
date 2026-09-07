#ifndef DEEP_PRESTIGE_H
#define DEEP_PRESTIGE_H

#include "bignum.h"

typedef enum {
    PRESTIGE_NONE = 0,
    PRESTIGE_PLANETARY_EVACUATION,
    PRESTIGE_SYSTEM_MIGRATION,
    PRESTIGE_INTERSTELLAR_MIGRATION,
    PRESTIGE_GALACTIC_EXPANSION
} PrestigeTier;

typedef struct {
    PrestigeTier tier;
    int evacuation_count;
    BigNum insight;            /* permanent currency retained across resets */
    double mining_bonus_mult;  /* permanent multiplier earned from past evacuations */
} PrestigeState;

void prestige_state_init(PrestigeState *ps);
BigNum prestige_calc_insight_gain(double max_depth_reached, int research_completed);

#endif
