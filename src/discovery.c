#include "discovery.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>

/* Discoveries are ordered so that, as a rough rule, the lower-index (more
 * common, shallow) ones are found early and the strange ones only reveal
 * themselves once the player has pushed into layers that shouldn't exist. */
void discovery_state_init(DiscoveryState *ds) {
    memset(ds, 0, sizeof(*ds));
    int n = 0;
    Discovery *d;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Buried Debris", sizeof(d->name) - 1);
    strncpy(d->text, "Sensors detect fragments of processed metal buried far below any known settlement. Age unknown.", sizeof(d->text) - 1);
    d->min_layer = 1; d->rarity = 0.05; d->discovered = 0; d->planet_specific = -1;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Fossilized Structure", sizeof(d->name) - 1);
    strncpy(d->text, "A geometrically regular formation, far too symmetrical to have formed naturally.", sizeof(d->text) - 1);
    d->min_layer = 2; d->rarity = 0.04; d->discovered = 0; d->planet_specific = -1;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Ancient Structures", sizeof(d->name) - 1);
    strncpy(d->text, "Beneath the Martian dust: cut stone bearing no resemblance to any known geological process.", sizeof(d->text) - 1);
    d->min_layer = 1; d->rarity = 0.06; d->discovered = 0; d->planet_specific = 2; /* Mars */

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Subsurface Ocean Current", sizeof(d->name) - 1);
    strncpy(d->text, "Europa's hidden ocean churns with currents that do not match any tidal model.", sizeof(d->text) - 1);
    d->min_layer = 2; d->rarity = 0.06; d->discovered = 0; d->planet_specific = 4; /* Europa */

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Unidentified Organism", sizeof(d->name) - 1);
    strncpy(d->text, "A motile sample recovered from Europa's ocean does not match any known biochemistry.", sizeof(d->text) - 1);
    d->min_layer = 3; d->rarity = 0.03; d->discovered = 0; d->planet_specific = 4;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Anomalous Signal", sizeof(d->name) - 1);
    strncpy(d->text, "A repeating signal, structured and non-random, originating from somewhere within the mantle.", sizeof(d->text) - 1);
    d->min_layer = 3; d->rarity = 0.025; d->discovered = 0; d->planet_specific = -1;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Artificial Alloy", sizeof(d->name) - 1);
    strncpy(d->text, "This material's composition matches no natural or known synthetic process. It appears... manufactured.", sizeof(d->text) - 1);
    d->min_layer = 4; d->rarity = 0.025; d->discovered = 0; d->planet_specific = -1;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Object Older Than The Planet", sizeof(d->name) - 1);
    strncpy(d->text, "Isotopic dating places this object several billion years before the planet itself formed. That should not be possible.", sizeof(d->text) - 1);
    d->min_layer = 4; d->rarity = 0.02; d->discovered = 0; d->planet_specific = -1;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Impossible Geological Formation", sizeof(d->name) - 1);
    strncpy(d->text, "A crystalline lattice at this depth and pressure should be unstable. It is not just stable -- it is growing.", sizeof(d->text) - 1);
    d->min_layer = 5; d->rarity = 0.025; d->discovered = 0; d->planet_specific = -1;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Structure Deeper Than The Planet Allows", sizeof(d->name) - 1);
    strncpy(d->text, "By planetary radius, this depth should place you outside the planet entirely. The instruments disagree with geometry itself.", sizeof(d->text) - 1);
    d->min_layer = 6; d->rarity = 0.03; d->discovered = 0; d->planet_specific = -1;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Negative Density Reading", sizeof(d->name) - 1);
    strncpy(d->text, "The material here registers a negative density. The instruments have been recalibrated three times. The reading persists.", sizeof(d->text) - 1);
    d->min_layer = 6; d->rarity = 0.025; d->discovered = 0; d->planet_specific = -1;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "The Boundary That Isn't There", sizeof(d->name) - 1);
    strncpy(d->text, "There is no core boundary. There was never a core boundary. You have been descending through something else entirely.", sizeof(d->text) - 1);
    d->min_layer = 6; d->rarity = 0.015; d->discovered = 0; d->planet_specific = -1;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Cavern Echo", sizeof(d->name) - 1);
    strncpy(d->text, "The caverns beneath Luna's maria echo back sounds that were never made. The delay does not match any geometry the tunnels actually have.", sizeof(d->text) - 1);
    d->min_layer = 2; d->rarity = 0.04; d->discovered = 0; d->planet_specific = 1; /* Luna */

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Engineered Stability", sizeof(d->name) - 1);
    strncpy(d->text, "A crystalline structure on Venus should be destroyed instantly at this temperature and pressure. It isn't. Something is holding it together on purpose.", sizeof(d->text) - 1);
    d->min_layer = 4; d->rarity = 0.03; d->discovered = 0; d->planet_specific = 3; /* Venus */

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Frozen Transmission", sizeof(d->name) - 1);
    strncpy(d->text, "A pattern is frozen into Titan's methane ice that resembles a data structure more than a natural formation. It has not thawed once in the estimated million years it has been here.", sizeof(d->text) - 1);
    d->min_layer = 3; d->rarity = 0.035; d->discovered = 0; d->planet_specific = 5; /* Titan */

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "Foreign Geometry", sizeof(d->name) - 1);
    strncpy(d->text, "The crust here follows a geometry that does not fully resolve in three dimensions. Survey instruments keep returning to the same coordinate from different directions.", sizeof(d->text) - 1);
    d->min_layer = 1; d->rarity = 0.05; d->discovered = 0; d->planet_specific = -1; /* triggers on any interstellar world */

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "The Second Core", sizeof(d->name) - 1);
    strncpy(d->text, "Gravimetric sensors detect a second center of mass where the planet's own core should be the only one. Neither core appears to notice the other.", sizeof(d->text) - 1);
    d->min_layer = 5; d->rarity = 0.02; d->discovered = 0; d->planet_specific = -1;

    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "A Voice, Waiting", sizeof(d->name) - 1);
    strncpy(d->text, "At the very edge of what the instruments can register, something responds to the drill. Not an echo. A reply.", sizeof(d->text) - 1);
    d->min_layer = 6; d->rarity = 0.01; d->discovered = 0; d->planet_specific = -1;

    /* This one is never found by chance. It is only unlocked by Dimensional
     * Surveying research synthesising everything found before it - the
     * moment the research tree actually explains the mystery, rather than
     * just cataloguing more of it. */
    d = &ds->items[n]; d->id = n; n++;
    strncpy(d->name, "The Pattern Emerges", sizeof(d->name) - 1);
    strncpy(d->text,
        "Dimensional Surveying cross-references every anomaly logged so far, and a shape resolves: the debris, the "
        "signals, the structures older than their worlds, the boundaries that were never really there. None of it "
        "was scattered. Every 'impossible' reading was a piece of the same much larger thing, and you have been "
        "excavating pieces of it on every planet you have ever touched.",
        sizeof(d->text) - 1);
    d->min_layer = 0; d->rarity = 0.0; d->discovered = 0; d->planet_specific = -1;

    ds->count = n;
    ds->total_discovered = 0;
}

Discovery *discovery_try_roll(DiscoveryState *ds, int planet_id, int layer, double scanner_power) {
    int i;
    if (scanner_power <= 0.0) scanner_power = 1.0;
    for (i = 0; i < ds->count; i++) {
        Discovery *d = &ds->items[i];
        if (d->discovered) continue;
        if (d->rarity <= 0.0) continue; /* not found by random chance */
        if (d->planet_specific != -1 && d->planet_specific != planet_id) continue;
        if (layer < d->min_layer) continue;
        double chance = d->rarity * scanner_power;
        double roll = (double)rand() / (double)RAND_MAX;
        if (roll < chance) {
            d->discovered = 1;
            ds->total_discovered++;
            return d;
        }
    }
    return NULL;
}

Discovery *discovery_find(DiscoveryState *ds, const char *name) {
    int i;
    for (i = 0; i < ds->count; i++) {
        if (strcasecmp(ds->items[i].name, name) == 0) return &ds->items[i];
    }
    return NULL;
}
