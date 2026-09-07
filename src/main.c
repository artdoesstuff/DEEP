#include "game.h"
#include "cli.h"
#include "save.h"
#include "simulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    srand((unsigned int)time(NULL));

    GameState g;
    if (load_game(&g, "deep_save.dat") == 0) {
        long elapsed = (long)difftime(time(NULL), g.last_tick_time);
        printf("Existing operation found. Resuming...\n");
        if (elapsed > 5) {
            simulation_apply_offline_progress(&g, elapsed);
            printf("Time elapsed since last session: %ld seconds. Any active stations kept working.\n", elapsed);
        }
    } else {
        game_new(&g);
        printf("No existing save found. Initializing new mining operation.\n");
    }

    cli_run(&g);
    return 0;
}
