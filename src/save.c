#include "save.h"
#include <stdio.h>
#include <string.h>

int save_game(const GameState *g, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    GameState copy = *g;
    copy.last_tick_time = time(NULL);
    size_t written = fwrite(&copy, sizeof(GameState), 1, f);
    fclose(f);
    return written == 1 ? 0 : -2;
}

int load_game(GameState *g, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    GameState tmp;
    size_t r = fread(&tmp, sizeof(GameState), 1, f);
    fclose(f);
    if (r != 1) return -2;
    if (tmp.magic != SAVE_MAGIC || tmp.version != SAVE_VERSION) return -3;
    *g = tmp;
    return 0;
}
