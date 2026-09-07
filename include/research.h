#ifndef DEEP_RESEARCH_H
#define DEEP_RESEARCH_H

#include "bignum.h"
#include "resources.h"

#define MAX_RESEARCH 32

typedef enum {
    RESEARCH_ADVANCED_DRILLING = 0,
    RESEARCH_AUTONOMOUS_MINING,
    RESEARCH_ION_PROPULSION,
    RESEARCH_DEEP_CORE_STABILIZATION,
    RESEARCH_ATMOSPHERIC_EXTRACTION,
    RESEARCH_QUANTUM_SCANNING,
    RESEARCH_FUSION_PROPULSION,
    RESEARCH_EXOTIC_MATTER_RESEARCH,
    RESEARCH_ANTIMATTER_CONTAINMENT,
    RESEARCH_DIMENSIONAL_SURVEYING,
    RESEARCH_PLANETARY_INFRASTRUCTURE,
    RESEARCH_ORBITAL_LOGISTICS,
    RESEARCH_STAR_SYSTEM_INDUSTRY,
    RESEARCH_COUNT
} ResearchId;

typedef struct {
    ResearchId id;
    char name[48];
    char description[200];
    BigNum cost[RES_COUNT];
    int prereq_count;
    ResearchId prereqs[3];
    int min_depth_layer; /* LayerType that must be reached somewhere first, -1 = none */
    int completed;
} ResearchNode;

typedef struct {
    ResearchNode nodes[MAX_RESEARCH];
    int count;
} ResearchState;

void research_state_init(ResearchState *rs);
ResearchNode *research_find(ResearchState *rs, const char *name_or_id);
int research_is_unlocked_prereqs(const ResearchState *rs, const ResearchNode *n);
int research_has(const ResearchState *rs, ResearchId id);

#endif
