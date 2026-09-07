#include "research.h"
#include "planet.h"
#include <string.h>
#include <strings.h>

static void zero_cost(BigNum c[RES_COUNT]) {
    int i;
    for (i = 0; i < RES_COUNT; i++) c[i] = bn_zero();
}

void research_state_init(ResearchState *rs) {
    memset(rs, 0, sizeof(*rs));
    int n = 0;
    ResearchNode *r;

    r = &rs->nodes[n++];
    r->id = RESEARCH_ADVANCED_DRILLING;
    strncpy(r->name, "Advanced Drilling", sizeof(r->name) - 1);
    strncpy(r->description, "Improved drill heads increase mining speed across all depths.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_STEEL] = bn_from_double(15);
    r->cost[RES_MACHINE_COMPONENTS] = bn_from_double(5);
    r->prereq_count = 0; r->min_depth_layer = -1; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_AUTONOMOUS_MINING;
    strncpy(r->name, "Autonomous Mining", sizeof(r->name) - 1);
    strncpy(r->description, "Deploy automated drilling rigs that keep extracting resources even while you are away.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_MACHINE_COMPONENTS] = bn_from_double(20);
    r->cost[RES_MINING_EQUIPMENT] = bn_from_double(3);
    r->prereq_count = 1; r->prereqs[0] = RESEARCH_ADVANCED_DRILLING; r->min_depth_layer = -1; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_ION_PROPULSION;
    strncpy(r->name, "Ion Propulsion", sizeof(r->name) - 1);
    strncpy(r->description, "Ion drives cut fuel consumption dramatically and open the way to Mars and Venus.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_TITANIUM] = bn_from_double(40);
    r->cost[RES_MACHINE_COMPONENTS] = bn_from_double(20);
    r->prereq_count = 0; r->min_depth_layer = -1; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_DEEP_CORE_STABILIZATION;
    strncpy(r->name, "Deep-Core Stabilization", sizeof(r->name) - 1);
    strncpy(r->description, "Structural field generators allow safe operation within planetary mantles and cores.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_TITANIUM] = bn_from_double(200);
    r->cost[RES_RARE_METALS] = bn_from_double(80);
    r->prereq_count = 1; r->prereqs[0] = RESEARCH_ADVANCED_DRILLING; r->min_depth_layer = LAYER_DEEP_CRUST; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_ATMOSPHERIC_EXTRACTION;
    strncpy(r->name, "Atmospheric Extraction", sizeof(r->name) - 1);
    strncpy(r->description, "Harvest resources directly from planetary atmospheres, unlocking Titan's hydrocarbon clouds.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_MACHINE_COMPONENTS] = bn_from_double(60);
    r->cost[RES_HYDROCARBONS] = bn_from_double(30);
    r->prereq_count = 0; r->min_depth_layer = -1; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_QUANTUM_SCANNING;
    strncpy(r->name, "Quantum Scanning", sizeof(r->name) - 1);
    strncpy(r->description, "Quantum-entangled sensors sharply increase discovery odds and reveal hidden phenomena.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_RARE_METALS] = bn_from_double(100);
    r->cost[RES_MACHINE_COMPONENTS] = bn_from_double(60);
    r->prereq_count = 0; r->min_depth_layer = -1; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_FUSION_PROPULSION;
    strncpy(r->name, "Fusion Propulsion", sizeof(r->name) - 1);
    strncpy(r->description, "Fusion drives unlock travel to the outer system: Europa and Titan.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_HELIUM3] = bn_from_double(150);
    r->cost[RES_TITANIUM] = bn_from_double(100);
    r->prereq_count = 1; r->prereqs[0] = RESEARCH_ION_PROPULSION; r->min_depth_layer = -1; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_EXOTIC_MATTER_RESEARCH;
    strncpy(r->name, "Exotic Matter Research", sizeof(r->name) - 1);
    strncpy(r->description, "Begin to characterize the impossible materials pulled from extreme depths.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_UNKNOWN_MATERIAL] = bn_from_double(20);
    r->cost[RES_RARE_METALS] = bn_from_double(150);
    r->prereq_count = 1; r->prereqs[0] = RESEARCH_DEEP_CORE_STABILIZATION; r->min_depth_layer = LAYER_CORE; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_ANTIMATTER_CONTAINMENT;
    strncpy(r->name, "Antimatter Containment", sizeof(r->name) - 1);
    strncpy(r->description, "Stable magnetic bottles enable antimatter propulsion, opening a path to Proxima Centauri.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_EXOTIC_MATTER] = bn_from_double(30);
    r->cost[RES_MACHINE_COMPONENTS] = bn_from_double(200);
    r->prereq_count = 1; r->prereqs[0] = RESEARCH_EXOTIC_MATTER_RESEARCH; r->min_depth_layer = -1; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_DIMENSIONAL_SURVEYING;
    strncpy(r->name, "Dimensional Surveying", sizeof(r->name) - 1);
    strncpy(r->description, "Instruments capable of mapping structures that do not fully exist within normal spacetime.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_EXOTIC_MATTER] = bn_from_double(80);
    r->cost[RES_UNKNOWN_MATERIAL] = bn_from_double(150);
    r->prereq_count = 1; r->prereqs[0] = RESEARCH_QUANTUM_SCANNING; r->min_depth_layer = LAYER_UNKNOWN; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_PLANETARY_INFRASTRUCTURE;
    strncpy(r->name, "Planetary Infrastructure", sizeof(r->name) - 1);
    strncpy(r->description, "Permanent surface infrastructure. Lets you build a mining station that keeps working a planet even when your ship is elsewhere.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_MACHINE_COMPONENTS] = bn_from_double(40);
    r->cost[RES_STEEL] = bn_from_double(60);
    r->prereq_count = 1; r->prereqs[0] = RESEARCH_AUTONOMOUS_MINING; r->min_depth_layer = -1; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_ORBITAL_LOGISTICS;
    strncpy(r->name, "Orbital Logistics", sizeof(r->name) - 1);
    strncpy(r->description, "Automated cargo shuttles link every station directly to your central stockpile and expand its capacity.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_MACHINE_COMPONENTS] = bn_from_double(200);
    r->cost[RES_TITANIUM] = bn_from_double(150);
    r->prereq_count = 1; r->prereqs[0] = RESEARCH_PLANETARY_INFRASTRUCTURE; r->min_depth_layer = -1; r->completed = 0;

    r = &rs->nodes[n++];
    r->id = RESEARCH_STAR_SYSTEM_INDUSTRY;
    strncpy(r->name, "Star-System Industry", sizeof(r->name) - 1);
    strncpy(r->description, "Coordinate every station in a system as a single industrial network, boosting all of their output.", sizeof(r->description) - 1);
    zero_cost(r->cost);
    r->cost[RES_RARE_METALS] = bn_from_double(300);
    r->cost[RES_MACHINE_COMPONENTS] = bn_from_double(300);
    r->cost[RES_EXOTIC_MATTER] = bn_from_double(10);
    r->prereq_count = 1; r->prereqs[0] = RESEARCH_ORBITAL_LOGISTICS; r->min_depth_layer = -1; r->completed = 0;

    rs->count = n;
}

ResearchNode *research_find(ResearchState *rs, const char *name_or_id) {
    int i;
    for (i = 0; i < rs->count; i++) {
        if (strcasecmp(rs->nodes[i].name, name_or_id) == 0) return &rs->nodes[i];
    }
    for (i = 0; i < rs->count; i++) {
        if (strncasecmp(rs->nodes[i].name, name_or_id, strlen(name_or_id)) == 0) return &rs->nodes[i];
    }
    return NULL;
}

int research_is_unlocked_prereqs(const ResearchState *rs, const ResearchNode *n) {
    int i, j;
    for (i = 0; i < n->prereq_count; i++) {
        int found = 0;
        for (j = 0; j < rs->count; j++) {
            if (rs->nodes[j].id == n->prereqs[i] && rs->nodes[j].completed) { found = 1; break; }
        }
        if (!found) return 0;
    }
    return 1;
}

int research_has(const ResearchState *rs, ResearchId id) {
    int i;
    for (i = 0; i < rs->count; i++) {
        if (rs->nodes[i].id == id) return rs->nodes[i].completed;
    }
    return 0;
}
