#include "prestige.h"
#include <math.h>

void prestige_state_init(PrestigeState *ps) {
    ps->tier = PRESTIGE_NONE;
    ps->evacuation_count = 0;
    ps->insight = bn_zero();
    ps->mining_bonus_mult = 1.0;
}

BigNum prestige_calc_insight_gain(double max_depth_reached, int research_completed) {
    if (max_depth_reached < 1000.0) return bn_zero();
    double base = sqrt(max_depth_reached) / 50.0;
    base *= (1.0 + research_completed * 0.2);
    return bn_from_double(base);
}
